/*
  ==============================================================================
    DrumPadComponent.h
    A 4x4 grid of drum pads. Each pad:
      - Highlights on mouse click and triggers its sample
      - Lights up and plays when a MIDI note-on is received on that pad's note
      - Loads pad_0.wav … pad_15.wav from the Samples/ folder next to the app

    CONCEPT: MidiInputCallback::handleIncomingMidiMessage() is called on a
             background MIDI thread — we must NOT do audio work or UI updates
             directly. We post to the message thread via MessageManager.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SamplePlayer.h"

//==============================================================================
// A single pad button — a coloured square that highlights when active
class PadButton : public juce::Component
{
public:
    explicit PadButton (int index);

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp   (const juce::MouseEvent&) override;

    // Called from MIDI thread via callAsync — sets visual state
    void highlight (bool shouldHighlight);

    std::function<void()> onTriggered; // called when the pad fires

private:
    int  padIndex;
    bool isHighlighted = false;
    juce::Colour padColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PadButton)
};

//==============================================================================
class DrumPadComponent : public juce::Component,
                         public juce::MidiInputCallback  // ← MIDI thread callback
{
public:
    explicit DrumPadComponent (juce::AudioDeviceManager& deviceManager);
    ~DrumPadComponent() override;

    void paint  (juce::Graphics& g) override;
    void resized() override;

private:
    // -------------------------------------------------------------------------
    // MidiInputCallback interface — called on the MIDI background thread
    // -------------------------------------------------------------------------
    void handleIncomingMidiMessage (juce::MidiInput* source,
                                    const juce::MidiMessage& message) override;

    // Called when a pad should fire (from either mouse or MIDI)
    void triggerPad (int padIndex);

    juce::AudioDeviceManager& deviceManager;

    // -------------------------------------------------------------------------
    // CONCEPT: MixerAudioSource lets us play multiple samples simultaneously
    // by mixing their outputs before sending to the device.
    // -------------------------------------------------------------------------
    juce::MixerAudioSource            mixer;
    juce::AudioSourcePlayer           audioSourcePlayer;

    static constexpr int kNumPads = 16;

    // 16 pads and 16 sample players
    std::array<std::unique_ptr<PadButton>,    kNumPads> pads;
    std::array<std::unique_ptr<SamplePlayer>, kNumPads> samplePlayers;

    // MIDI note numbers assigned to each pad (C2 … D#3 by default)
    std::array<int, kNumPads> padNotes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumPadComponent)
};
