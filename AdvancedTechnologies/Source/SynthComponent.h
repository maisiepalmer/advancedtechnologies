/*
  ==============================================================================
    SynthComponent.h
    The Synth tab UI.  Three sliders (Frequency, Volume, Attack) are backed by
    an AudioProcessorValueTreeState, demonstrating the full APVTS pattern
    outside of an AudioProcessor.

    CONCEPT: APVTS stores all parameters in a ValueTree. Sliders don't need
             Listener callbacks — SliderAttachment does the wiring for you.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SynthAudioSource.h"

class SynthComponent : public juce::Component
{
public:
    explicit SynthComponent (juce::AudioDeviceManager& deviceManager);
    ~SynthComponent() override;

    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    //--------------------------------------------------------------------------
    // APVTS needs a "dummy" AudioProcessor to satisfy its constructor.
    // For standalone teaching code we create a minimal one inline.
    //--------------------------------------------------------------------------
    struct DummyProcessor : public juce::AudioProcessor
    {
        DummyProcessor() : AudioProcessor (BusesProperties()
                               .withOutput ("Output", juce::AudioChannelSet::stereo(), true)) {}
        const juce::String getName() const override            { return "Dummy"; }
        void prepareToPlay (double, int) override              {}
        void releaseResources() override                       {}
        void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
        double getTailLengthSeconds() const override           { return 0.0; }
        bool acceptsMidi() const override                      { return false; }
        bool producesMidi() const override                     { return false; }
        juce::AudioProcessorEditor* createEditor() override    { return nullptr; }
        bool hasEditor() const override                        { return false; }
        int getNumPrograms() override                          { return 1; }
        int getCurrentProgram() override                       { return 0; }
        void setCurrentProgram (int) override                  {}
        const juce::String getProgramName (int) override       { return {}; }
        void changeProgramName (int, const juce::String&) override {}
        void getStateInformation (juce::MemoryBlock&) override {}
        void setStateInformation (const void*, int) override   {}
    };

    // -------------------------------------------------------------------------
    // CONCEPT: createParameterLayout() defines every parameter with its range,
    // default, and identifier string. This layout is stored in the ValueTree.
    // -------------------------------------------------------------------------
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // -------------------------------------------------------------------------
    // Member order matters! dummyProcessor must exist before apvts is
    // constructed, because APVTS takes a reference to an AudioProcessor.
    // -------------------------------------------------------------------------
    DummyProcessor                       dummyProcessor;
    juce::AudioProcessorValueTreeState   apvts;

    // The audio source that reads from apvts and generates the waveform
    SynthAudioSource                     audioSource;

    // Reference to the shared device manager (owned by MainComponent)
    juce::AudioDeviceManager&            deviceManager;

    //--------------------------------------------------------------------------
    // UI Controls
    //--------------------------------------------------------------------------
    juce::Slider frequencySlider;
    juce::Slider volumeSlider;
    juce::Slider attackSlider;   // attack shapes the envelope (educational — not wired to DSP in this skeleton)

    juce::Label  frequencyLabel;
    juce::Label  volumeLabel;
    juce::Label  attackLabel;

    // ToggleButton acts as a play/stop gate
    juce::TextButton playButton { "Play" };
    bool isPlaying = false;

    // -------------------------------------------------------------------------
    // CONCEPT: SliderAttachment keeps the Slider and the APVTS parameter in
    // sync bidirectionally with no extra listener code.
    // -------------------------------------------------------------------------
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> frequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;

    // AudioSourcePlayer wraps our AudioSource as an AudioIODeviceCallback
    // so it can be registered with the AudioDeviceManager
    juce::AudioSourcePlayer audioSourcePlayer;

    void setupSlider (juce::Slider& slider, juce::Label& label,
                      const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthComponent)
};
