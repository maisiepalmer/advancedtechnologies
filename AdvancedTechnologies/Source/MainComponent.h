/*
  ==============================================================================
    MainComponent.h
    Root component. Owns the AudioDeviceManager and hosts a TabbedComponent
    with the Synth and Drum Pad pages.

    CONCEPT: AudioDeviceManager is the bridge between your app and the OS audio
             hardware. One instance is shared across the whole app.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SynthComponent.h"
#include "DrumPadComponent.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //--------------------------------------------------------------------------
    // AudioDeviceManager: discovers and opens the system audio/MIDI hardware.
    // We create it here and pass references into child components.
    juce::AudioDeviceManager deviceManager;

    // TabbedComponent provides the tab strip at the top
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };

    // These are owned by the TabbedComponent after we add them
    SynthComponent*    synthPage    = nullptr;
    DrumPadComponent*  drumPadPage  = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
