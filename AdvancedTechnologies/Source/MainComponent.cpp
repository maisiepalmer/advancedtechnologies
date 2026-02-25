/*
  ==============================================================================
    MainComponent.cpp
  ==============================================================================
*/

#include "MainComponent.h"

MainComponent::MainComponent()
{
    // -------------------------------------------------------------------------
    // STEP 1: Initialise the audio device.
    // initialiseWithDefaultDevices() picks the system default input + output.
    // The arguments are: numInputChannels, numOutputChannels.
    // -------------------------------------------------------------------------
    deviceManager.initialiseWithDefaultDevices (0, 2); // stereo out, no input

    // -------------------------------------------------------------------------
    // STEP 2: Create our two pages.
    // We pass a reference to the deviceManager so each page can hook in.
    // -------------------------------------------------------------------------
    synthPage   = new SynthComponent   (deviceManager);
    drumPadPage = new DrumPadComponent (deviceManager);

    // -------------------------------------------------------------------------
    // STEP 3: Add them to the tab strip.
    // addTab() takes ownership of the component (last arg = true).
    // -------------------------------------------------------------------------
    tabs.addTab ("Synth",    juce::Colours::darkslategrey, synthPage,   true);
    tabs.addTab ("Drum Pad", juce::Colours::darkslategrey, drumPadPage, true);

    addAndMakeVisible (tabs);
    setSize (700, 500);
}

MainComponent::~MainComponent()
{
    // TabbedComponent owns its pages, so they are deleted automatically.
    // We just need to make sure audio is stopped before anything is destroyed.
    deviceManager.removeAllChangeListeners();
    deviceManager.closeAudioDevice();
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1e2e));
}

void MainComponent::resized()
{
    tabs.setBounds (getLocalBounds());
}
