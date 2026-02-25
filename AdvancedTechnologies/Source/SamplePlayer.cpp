/*
  ==============================================================================
    SamplePlayer.cpp
  ==============================================================================
*/

#include "SamplePlayer.h"

SamplePlayer::SamplePlayer()
{
    // Register the built-in JUCE decoders (WAV, AIFF, and more with the
    // right flags in the Projucer).
    formatManager.registerBasicFormats();
}

bool SamplePlayer::loadSample (const juce::File& file)
{
    // createReaderFor() tries all registered formats in order
    auto* reader = formatManager.createReaderFor (file);

    if (reader == nullptr)
        return false; // file not found or format not supported

    name = file.getFileNameWithoutExtension();

    // -------------------------------------------------------------------------
    // CONCEPT: AudioFormatReaderSource takes ownership of the reader.
    // The second argument (shouldDeleteReader) = true means it will be freed.
    // -------------------------------------------------------------------------
    readerSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);

    // Set the reader into the transport, using the reader's sample rate for resampling
    transportSource.setSource (readerSource.get(),
                               0,            // read-ahead buffer size (0 = synchronous)
                               nullptr,      // background thread (nullptr = synchronous)
                               reader->sampleRate);
    return true;
}

void SamplePlayer::trigger()
{
    // -------------------------------------------------------------------------
    // CONCEPT: setPosition(0) rewinds, then start() begins playback.
    // Both calls are safe from any thread once prepareToPlay has been called.
    // -------------------------------------------------------------------------
    transportSource.setPosition (0.0);
    transportSource.start();
}

void SamplePlayer::prepareToPlay (int samplesPerBlock, double sampleRate)
{
    transportSource.prepareToPlay (samplesPerBlock, sampleRate);
}

void SamplePlayer::releaseResources()
{
    transportSource.releaseResources();
}
