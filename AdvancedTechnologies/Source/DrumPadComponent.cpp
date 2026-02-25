/*
  ==============================================================================
    DrumPadComponent.cpp
  ==============================================================================
*/

#include "DrumPadComponent.h"

//==============================================================================
// PadButton
//==============================================================================

PadButton::PadButton (int index)
    : padIndex (index)
{
    // Give each pad a distinct but harmonious colour from a fixed palette
    const juce::Colour palette[] = {
        juce::Colour (0xffE63946), juce::Colour (0xffF4A261), juce::Colour (0xff2A9D8F), juce::Colour (0xff457B9D),
        juce::Colour (0xffE9C46A), juce::Colour (0xff8338EC), juce::Colour (0xffFB5607), juce::Colour (0xff06D6A0),
        juce::Colour (0xffFFBE0B), juce::Colour (0xff3A86FF), juce::Colour (0xffFF006E), juce::Colour (0xff8AC926),
        juce::Colour (0xff1982C4), juce::Colour (0xffFF595E), juce::Colour (0xff6A4C93), juce::Colour (0xffFFCA3A)
    };

    padColour = palette[index % 16];
    setRepaintsOnMouseActivity (true);
}

void PadButton::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (4.0f);

    // Background: dimmed when idle, bright when highlighted
    g.setColour (isHighlighted ? padColour : padColour.darker (0.6f));
    g.fillRoundedRectangle (bounds, 8.0f);

    // Subtle border
    g.setColour (isHighlighted ? juce::Colours::white.withAlpha (0.8f)
                               : juce::Colours::white.withAlpha (0.15f));
    g.drawRoundedRectangle (bounds, 8.0f, 1.5f);

    // Pad number label
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.setFont (12.0f);
    g.drawText (juce::String (padIndex + 1), bounds, juce::Justification::centred);
}

void PadButton::mouseDown (const juce::MouseEvent&)
{
    highlight (true);
    if (onTriggered) onTriggered();
}

void PadButton::mouseUp (const juce::MouseEvent&)
{
    // Use a short timer to keep the highlight visible briefly
    juce::Timer::callAfterDelay (80, [this] { highlight (false); });
}

void PadButton::highlight (bool shouldHighlight)
{
    isHighlighted = shouldHighlight;
    repaint();
}

//==============================================================================
// DrumPadComponent
//==============================================================================

DrumPadComponent::DrumPadComponent (juce::AudioDeviceManager& dm)
    : deviceManager (dm)
{
    // -------------------------------------------------------------------------
    // Assign MIDI notes: start from C2 (MIDI note 36) going chromatically
    // -------------------------------------------------------------------------
    for (int i = 0; i < kNumPads; ++i)
        padNotes[i] = 36 + i;

    // -------------------------------------------------------------------------
    // Create pads and sample players
    // -------------------------------------------------------------------------
    juce::File samplesDir = juce::File::getSpecialLocation (
                                juce::File::currentExecutableFile)
                            .getParentDirectory()
                            .getChildFile ("Samples");

    for (int i = 0; i < kNumPads; ++i)
    {
        // Create the pad button
        pads[i] = std::make_unique<PadButton> (i);
        const int padIndex = i; // capture by value
        pads[i]->onTriggered = [this, padIndex] { triggerPad (padIndex); };
        addAndMakeVisible (*pads[i]);

        // Create and load the sample player
        samplePlayers[i] = std::make_unique<SamplePlayer>();

        auto sampleFile = samplesDir.getChildFile ("pad_" + juce::String (i) + ".wav");
        if (sampleFile.existsAsFile())
        {
            samplePlayers[i]->loadSample (sampleFile);
            // Add its AudioSource to the mixer
            mixer.addInputSource (samplePlayers[i]->getAudioSource(), false);
        }
        else
        {
            // No sample file — pad will produce silence but still light up
            DBG ("SamplePlayer " << i << ": file not found: " << sampleFile.getFullPathName());
        }
    }

    // -------------------------------------------------------------------------
    // CONCEPT: AudioSourcePlayer wraps our mixer as an AudioIODeviceCallback
    // and handles the prepareToPlay/releaseResources lifecycle automatically.
    // -------------------------------------------------------------------------
    audioSourcePlayer.setSource (&mixer);
    deviceManager.addAudioCallback (&audioSourcePlayer);

    // -------------------------------------------------------------------------
    // CONCEPT: addMidiInputDeviceCallback registers us for MIDI events on ALL
    // available MIDI inputs. The empty string "" means "all devices".
    // handleIncomingMidiMessage() will be called on a background MIDI thread.
    // -------------------------------------------------------------------------
    deviceManager.addMidiInputDeviceCallback ({}, this);

    setSize (700, 450);
}

DrumPadComponent::~DrumPadComponent()
{
    // Always deregister callbacks before the object is destroyed
    deviceManager.removeMidiInputDeviceCallback ({}, this);
    deviceManager.removeAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (nullptr);
}

void DrumPadComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1e2e));

    g.setColour (juce::Colours::lightblue);
    g.setFont (22.0f);
    g.drawText ("Drum Pad",
                getLocalBounds().removeFromTop (44),
                juce::Justification::centred);

    g.setColour (juce::Colours::grey);
    g.setFont (11.0f);
    g.drawText ("MIDI notes C2 (36) D#3 (51)  place pad_0.wav pad_15.wav in Samples/",
                getLocalBounds().removeFromBottom (22),
                juce::Justification::centred);
}

void DrumPadComponent::resized()
{
    auto area = getLocalBounds().reduced (12);
    area.removeFromTop  (44);  // title
    area.removeFromBottom (22); // note

    const int cols   = 4;
    const int rows   = 4;
    const int padW   = area.getWidth()  / cols;
    const int padH   = area.getHeight() / rows;

    for (int row = 0; row < rows; ++row)
        for (int col = 0; col < cols; ++col)
        {
            int idx = row * cols + col;
            pads[idx]->setBounds (area.getX() + col * padW,
                                  area.getY() + row * padH,
                                  padW, padH);
        }
}

//------------------------------------------------------------------------------
// MidiInputCallback — runs on the MIDI background thread
//------------------------------------------------------------------------------
void DrumPadComponent::handleIncomingMidiMessage (juce::MidiInput* /*source*/,
                                                   const juce::MidiMessage& message)
{
    // -------------------------------------------------------------------------
    // CONCEPT: isNoteOn() checks the status byte (0x9n) and velocity > 0.
    // We look up which pad this note belongs to using our padNotes map.
    // -------------------------------------------------------------------------
    if (! message.isNoteOn())
        return;

    const int note = message.getNoteNumber();

    for (int i = 0; i < kNumPads; ++i)
    {
        if (padNotes[i] == note)
        {
            // ---------------------------------------------------------------
            // CONCEPT: We MUST NOT update UI from the MIDI thread.
            // callAsync() posts a lambda to the message thread safely.
            // ---------------------------------------------------------------
            const int padIndex = i;
            juce::MessageManager::callAsync ([this, padIndex]
            {
                triggerPad (padIndex);

                // Visual flash: highlight briefly then restore
                pads[padIndex]->highlight (true);
                juce::Timer::callAfterDelay (80, [this, padIndex]
                {
                    if (padIndex < kNumPads && pads[padIndex] != nullptr)
                        pads[padIndex]->highlight (false);
                });
            });

            break;
        }
    }
}

void DrumPadComponent::triggerPad (int padIndex)
{
    if (padIndex >= 0 && padIndex < kNumPads && samplePlayers[padIndex] != nullptr)
        samplePlayers[padIndex]->trigger();
}
