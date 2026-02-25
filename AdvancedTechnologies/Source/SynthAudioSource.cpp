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
    // Always clear first — never leave garbage in the buffer
    bufferToFill.clearActiveBufferRegion();

    if (! isPlaying)
        return;

    // -------------------------------------------------------------------------
    // CONCEPT: getRawParameterValue() is lock-free and safe on the audio thread.
    // It returns a std::atomic<float>* — we load() it once per block.
    // -------------------------------------------------------------------------
    const float frequency = apvts.getRawParameterValue ("frequency")->load();
    const float volume    = apvts.getRawParameterValue ("volume")->load();

    // Phase increment per sample: Δφ = 2π * f / fs
    const double phaseIncrement = juce::MathConstants<double>::twoPi
                                  * frequency / currentSampleRate;

    const int numSamples = bufferToFill.numSamples;
    const int startSample = bufferToFill.startSample;

    // Write to every output channel (typically L and R)
    for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
    {
        float* channelData = bufferToFill.buffer->getWritePointer (channel, startSample);
        double phase = currentPhase; // local copy so both channels stay in phase

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Pure sine wave: y(t) = A * sin(φ)
            channelData[sample] = volume * (float) std::sin (phase);

            phase += phaseIncrement;
            if (phase >= juce::MathConstants<double>::twoPi)
                phase -= juce::MathConstants<double>::twoPi; // wrap to avoid float drift
        }
    }

    // Advance the shared phase accumulator by the full block
    currentPhase += phaseIncrement * numSamples;
    if (currentPhase >= juce::MathConstants<double>::twoPi)
        currentPhase -= juce::MathConstants<double>::twoPi;
}

void SynthAudioSource::releaseResources()
{
    // Nothing to release for a pure oscillator
}

void SynthAudioSource::setPlaying (bool shouldPlay)
{
    // This is called from the UI thread. isPlaying is read on the audio thread,
    // but a single bool write is atomic on all platforms JUCE targets.
    isPlaying = shouldPlay;
}
