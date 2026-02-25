/*
  ==============================================================================
    SynthAudioSource.h
    A minimal sine-wave oscillator that reads its parameters from an
    AudioProcessorValueTreeState AND responds to MIDI note-on / note-off
    messages.

    CONCEPT: By implementing MidiInputCallback we receive MIDI events on a
             background thread. We must communicate with the audio thread
             safely -- here we use std::atomic<> members rather than locks,
             which is the lightest-weight approach for simple values.

    MIDI behaviour:
      - Note-on  : sets the oscillator frequency from the MIDI note number
                   (standard equal temperament: f = 440 * 2^((n-69)/12))
                   and starts playback.
      - Note-off : stops playback only if it matches the currently held note
                   (so fast re-triggers don't cut the sound early).
      - The Frequency slider in the UI now acts as a fine-tune offset in
                   semitones (+/- 24), combining parameter automation with MIDI.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class SynthAudioSource : public juce::AudioSource,
                         public juce::MidiInputCallback   // <-- MIDI thread callback
{
public:
    explicit SynthAudioSource (juce::AudioProcessorValueTreeState& apvts);

    // AudioSource interface
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // -------------------------------------------------------------------------
    // MidiInputCallback interface -- called on the MIDI background thread.
    // We update atomics so the audio thread picks up the change lock-free.
    // -------------------------------------------------------------------------
    void handleIncomingMidiMessage (juce::MidiInput* source,
                                    const juce::MidiMessage& message) override;

    // Called from the UI play button -- manual play/stop without MIDI
    void setPlaying (bool shouldPlay);

    // Returns the MIDI note number currently held (-1 if none)
    int getCurrentNote() const { return currentNote.load(); }

    // Optional callback fired on the message thread when the note changes.
    // Set this from SynthComponent to update the UI label.
    std::function<void(int note)> onNoteChanged;

private:
    juce::AudioProcessorValueTreeState& apvts;

    double currentSampleRate = 44100.0;
    double currentPhase      = 0.0;

    // -------------------------------------------------------------------------
    // CONCEPT: std::atomic<> lets the MIDI thread write and the audio thread
    // read without a mutex. Only use this for simple scalar values.
    // -------------------------------------------------------------------------
    std::atomic<float> midiFrequency { 440.0f }; // set by MIDI note-on
    std::atomic<bool>  isPlaying     { false };
    std::atomic<int>   currentNote   { -1 };      // -1 = no note held

    // Converts a MIDI note number to Hz using equal temperament
    static float midiNoteToHz (int note)
    {
        return 440.0f * std::pow (2.0f, (note - 69) / 12.0f);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthAudioSource)
};
