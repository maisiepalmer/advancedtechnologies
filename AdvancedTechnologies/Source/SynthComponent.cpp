/*
  ==============================================================================
    SynthComponent.cpp
  ==============================================================================
*/

#include "SynthComponent.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout SynthComponent::createParameterLayout()
{
    // -------------------------------------------------------------------------
    // CONCEPT: Each parameter has:
    //   - A string ID used to retrieve it from anywhere (getRawParameterValue)
    //   - A human-readable name shown in DAW automation lanes
    //   - A NormalisableRange defining min, max, and step size
    //   - A default value
    // -------------------------------------------------------------------------
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "frequency",          // parameterID
        "Frequency",          // parameter name
        juce::NormalisableRange<float> (20.0f, 2000.0f, 1.0f, 0.4f), // skewed so low freqs aren't cramped
        220.0f));             // default: A3

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "volume",
        "Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.5f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "attack",
        "Attack (ms)",
        juce::NormalisableRange<float> (1.0f, 2000.0f, 1.0f, 0.4f),
        10.0f));

    return layout;
}

//==============================================================================
SynthComponent::SynthComponent (juce::AudioDeviceManager& dm)
    : apvts (dummyProcessor, nullptr, "SynthState", createParameterLayout()),
      audioSource (apvts),
      deviceManager (dm)
{
    // -------------------------------------------------------------------------
    // CONCEPT: AudioSourcePlayer wraps an AudioSource for the device manager.
    // We use the device manager directly here for simplicity.
    // -------------------------------------------------------------------------

    // Set up the three sliders
    setupSlider (frequencySlider, frequencyLabel, "Frequency (Hz)");
    setupSlider (volumeSlider,    volumeLabel,    "Volume");
    setupSlider (attackSlider,    attackLabel,    "Attack (ms)");

    // -------------------------------------------------------------------------
    // CONCEPT: SliderAttachment connects a Slider to a named APVTS parameter.
    // Moving the slider updates the ValueTree; the audio thread reads via
    // getRawParameterValue() — no manual listener needed.
    // -------------------------------------------------------------------------
    frequencyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                          (apvts, "frequency", frequencySlider);
    volumeAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                          (apvts, "volume", volumeSlider);
    attackAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                          (apvts, "attack", attackSlider);

    // Play/Stop button
    playButton.onClick = [this]
    {
        isPlaying = ! isPlaying;
        audioSource.setPlaying (isPlaying);

        if (isPlaying)
        {
            playButton.setButtonText ("Stop");
        }
        else
        {
            playButton.setButtonText ("Play");
        }
    };

    addAndMakeVisible (playButton);

    // -------------------------------------------------------------------------
    // Wire the AudioSource into the device manager via AudioSourcePlayer.
    // AudioSourcePlayer implements AudioIODeviceCallback and delegates to our
    // AudioSource.
    // -------------------------------------------------------------------------
    audioSourcePlayer.setSource (&audioSource);
    deviceManager.addAudioCallback (&audioSourcePlayer);

    setSize (700, 400);
}

SynthComponent::~SynthComponent()
{
    // Always remove the callback before the audio source is destroyed
    deviceManager.removeAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (nullptr);
}

void SynthComponent::setupSlider (juce::Slider& slider, juce::Label& label,
                                  const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (14.0f));
    addAndMakeVisible (label);
}

void SynthComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1e2e));
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.fillRoundedRectangle (getLocalBounds().reduced (12).toFloat(), 12.0f);

    g.setColour (juce::Colours::lightblue);
    g.setFont (22.0f);
    g.drawText ("Simple Sine Synthesiser", getLocalBounds().removeFromTop (50),
                juce::Justification::centred);

    g.setColour (juce::Colours::grey);
    g.setFont (12.0f);
    g.drawText ("Sliders are backed by AudioProcessorValueTreeState",
                getLocalBounds().removeFromBottom (30),
                juce::Justification::centred);
}

void SynthComponent::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (50);  // title

    // Play button at top-right
    playButton.setBounds (area.removeFromTop (40).removeFromRight (120));
    area.removeFromTop (10);

    // Three knobs side by side
    const int knobW = area.getWidth() / 3;
    const int labelH = 24;
    const int knobH  = area.getHeight() - labelH - 20;

    auto knobArea = area;

    auto freqBounds = knobArea.removeFromLeft (knobW);
    frequencyLabel.setBounds (freqBounds.removeFromBottom (labelH));
    frequencySlider.setBounds (freqBounds);

    auto volBounds = knobArea.removeFromLeft (knobW);
    volumeLabel.setBounds (volBounds.removeFromBottom (labelH));
    volumeSlider.setBounds (volBounds);

    auto atkBounds = knobArea;
    attackLabel.setBounds (atkBounds.removeFromBottom (labelH));
    attackSlider.setBounds (atkBounds);
}


