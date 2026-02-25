/*
  ==============================================================================
    SamplePlayer.h
    Loads a single .wav file and plays it back on demand.

    CONCEPT: AudioFormatManager knows about all registered file formats.
             AudioFormatReaderSource wraps a reader for use as an AudioSource.
             We mix multiple SamplePlayers via a MixerAudioSource.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class SamplePlayer
{
public:
    SamplePlayer();

    // Load a sample from disk. Returns true on success.
    bool loadSample (const juce::File& file);

    // Trigger playback from the beginning (called from UI or MIDI thread)
    void trigger();

    // AudioSource interface — used by MixerAudioSource
    juce::AudioSource* getAudioSource() { return &transportSource; }

    // Must be called before playback starts
    void prepareToPlay (int samplesPerBlock, double sampleRate);
    void releaseResources();

    const juce::String& getName() const { return name; }

private:
    // -------------------------------------------------------------------------
    // CONCEPT: AudioFormatManager registers decoders (WAV, AIFF, etc.).
    //          createReaderFor() opens a file and returns an AudioFormatReader.
    //          AudioFormatReaderSource wraps a reader as an AudioSource.
    //          AudioTransportSource adds start/stop/seek on top.
    // -------------------------------------------------------------------------
    juce::AudioFormatManager        formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource      transportSource;

    juce::String name;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplePlayer)
};
