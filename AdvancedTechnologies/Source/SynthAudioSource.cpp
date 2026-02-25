/*
  ==============================================================================
    SynthAudioSource.cpp
  ==============================================================================
*/

#include "SynthAudioSource.h"

SynthAudioSource::SynthAudioSource (juce::AudioProcessorValueTreeState& apvts_)
    : apvts (apvts_)
{}

void SynthAudioSource::prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate)
{
    currentSampleRate = sampleRate;
    currentPhase = 0.0;
}

void SynthAudioSource::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    if (! isPlaying.load())
        return;

    // -------------------------------------------------------------------------
    // CONCEPT: We combine two frequency sources:
    //   1. midiFrequency  -- set atomically by the MIDI thread on note-on
    //   2. "detune" param -- the Frequency slider now offsets by +/- 24 semitones
    //
    // Semitone offset -> frequency multiplier: 2^(semitones/12)
    // This shows students how parameters and live MIDI can work together.
    // -------------------------------------------------------------------------
    const float baseMidiHz  = midiFrequency.load();
    const float detuneSemi  = apvts.getRawParameterValue ("frequency")->load(); // -24 .. +24
    const float finalHz     = baseMidiHz * std::pow (2.0f, detuneSemi / 12.0f);
    const float volume      = apvts.getRawParameterValue ("volume")->load();

    const double phaseIncrement = juce::MathConstants<double>::twoPi
                                  * finalHz / currentSampleRate;

    const int numSamples  = bufferToFill.numSamples;
    const int startSample = bufferToFill.startSample;

    for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
    {
        float* channelData = bufferToFill.buffer->getWritePointer (channel, startSample);
        double phase = currentPhase;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = volume * (float) std::sin (phase);
            phase += phaseIncrement;
            if (phase >= juce::MathConstants<double>::twoPi)
                phase -= juce::MathConstants<double>::twoPi;
        }
    }

    currentPhase += phaseIncrement * numSamples;
    if (currentPhase >= juce::MathConstants<double>::twoPi)
        currentPhase -= juce::MathConstants<double>::twoPi;
}

void SynthAudioSource::releaseResources() {}

//------------------------------------------------------------------------------
// MidiInputCallback -- runs on the MIDI background thread
//------------------------------------------------------------------------------
void SynthAudioSource::handleIncomingMidiMessage (juce::MidiInput* /*source*/,
                                                   const juce::MidiMessage& message)
{
    // -------------------------------------------------------------------------
    // CONCEPT: isNoteOn() returns true for status byte 0x9n with velocity > 0.
    // isNoteOff() catches both 0x8n messages AND 0x9n with velocity == 0
    // (the latter is how many keyboards send note-off).
    // -------------------------------------------------------------------------
    if (message.isNoteOn())
    {
        const int note = message.getNoteNumber();
        midiFrequency.store (midiNoteToHz (note));  // atomic write -- audio thread safe
        currentNote.store (note);
        isPlaying.store (true);

        // Post a UI update to show the note name on the message thread
        juce::MessageManager::callAsync ([this, note]
        {
            if (onNoteChanged) onNoteChanged (note);
        });
    }
    else if (message.isNoteOff())
    {
        // Only stop if this is the note we're currently holding
        if (message.getNoteNumber() == currentNote.load())
        {
            isPlaying.store (false);
            currentNote.store (-1);

            juce::MessageManager::callAsync ([this]
            {
                if (onNoteChanged) onNoteChanged (-1);
            });
        }
    }
}

void SynthAudioSource::setPlaying (bool shouldPlay)
{
    isPlaying.store (shouldPlay);
    if (! shouldPlay)
        currentNote.store (-1);
}
