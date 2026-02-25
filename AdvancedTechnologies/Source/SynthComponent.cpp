/*
  ==============================================================================
    SynthComponent.cpp
  ==============================================================================
*/

#include "SynthComponent.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout SynthComponent::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // -------------------------------------------------------------------------
    // "frequency" is now a semitone detune offset (-24 .. +24).
    // The base pitch comes from MIDI. Default 0 = no detune.
    // -------------------------------------------------------------------------
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "frequency",
        "Detune (semitones)",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f),
        0.0f));

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
    // CONCEPT: onNoteChanged is a std::function set here on the UI thread.
    // It is called via MessageManager::callAsync so it always runs on the
    // message thread -- safe to update UI from here.
    // -------------------------------------------------------------------------
    audioSource.onNoteChanged = [this] (int note)
    {
        if (note < 0)
        {
            midiNoteLabel.setText ("--", juce::dontSendNotification);
            midiNoteLabel.setColour (juce::Label::textColourId, juce::Colours::grey);
        }
        else
        {
            const juce::String noteName = juce::MidiMessage::getMidiNoteName (note, true, true, 4);
            midiNoteLabel.setText (noteName + "  (MIDI " + juce::String (note) + ")",
                                   juce::dontSendNotification);
            midiNoteLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
        }
    };

    // Set up sliders
    setupSlider (detuneSlider, detuneLabel, "Detune (semitones)");
    setupSlider (volumeSlider, volumeLabel, "Volume");
    setupSlider (attackSlider, attackLabel, "Attack (ms)");

    detuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                       (apvts, "frequency", detuneSlider);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                       (apvts, "volume", volumeSlider);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                       (apvts, "attack", attackSlider);

    // -------------------------------------------------------------------------
    // Register audioSource as a MIDI callback -- it now receives note events
    // directly from the device manager on the MIDI background thread.
    // -------------------------------------------------------------------------
    deviceManager.addMidiInputDeviceCallback ({}, &audioSource);

    // Hook audio into device
    audioSourcePlayer.setSource (&audioSource);
    deviceManager.addAudioCallback (&audioSourcePlayer);

    setSize (700, 420);
}

SynthComponent::~SynthComponent()
{
    deviceManager.removeMidiInputDeviceCallback ({}, &audioSource);
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
    label.setFont (juce::Font (13.0f));
    addAndMakeVisible (label);
}

void SynthComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1e2e));
}

void SynthComponent::resized()
{
    auto area = getLocalBounds().reduced (20);

    // MIDI note display
    midiNoteLabel.setBounds (area.removeFromTop (36));
    area.removeFromTop (16);

    // Three knobs
    const int knobW  = area.getWidth() / 3;
    const int labelH = 24;

    auto detuneB = area.removeFromLeft (knobW);
    detuneLabel.setBounds (detuneB.removeFromBottom (labelH));
    detuneSlider.setBounds (detuneB);

    auto volB = area.removeFromLeft (knobW);
    volumeLabel.setBounds (volB.removeFromBottom (labelH));
    volumeSlider.setBounds (volB);

    attackLabel.setBounds (area.removeFromBottom (labelH));
    attackSlider.setBounds (area);
}


