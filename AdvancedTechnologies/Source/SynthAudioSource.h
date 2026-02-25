/*
  ==============================================================================
    SynthAudioSource.h
    A minimal sine-wave oscillator that reads its parameters from an
    AudioProcessorValueTreeState.

    CONCEPT: AudioSource is the pull-model audio graph node. The device manager
             calls getNextAudioBlock() on the audio thread — no UI work here!
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class SynthAudioSource : public juce::AudioSource
{
public:
    //--------------------------------------------------------------------------
    // Constructor — we store a reference to the APVTS so we can read parameter
    // values on the audio thread without allocating.
    //--------------------------------------------------------------------------
    explicit SynthAudioSource (juce::AudioProcessorValueTreeState& apvts);

    // AudioSource interface
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // Called from the UI thread (button click) to start/stop sound
    void setPlaying (bool shouldPlay);

private:
    juce::AudioProcessorValueTreeState& apvts;

    double currentSampleRate = 44100.0;
    double currentPhase      = 0.0;   // phase accumulator (0 … 2π)
    bool   isPlaying         = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthAudioSource)
};
