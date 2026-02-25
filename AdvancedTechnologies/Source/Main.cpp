/*
  ==============================================================================
    Main.cpp
    Entry point for the Advanced Technologies.

    CONCEPT: JUCEApplication is the top-level singleton that owns the window.
             It mirrors how a plugin's createPluginFilter() works for standalone.
  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

//==============================================================================
class AdvancedTechnologiesApplication : public juce::JUCEApplication
{
public:
    AdvancedTechnologiesApplication() {}

    const juce::String getApplicationName() override    { return "Advanced Technologies"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override          { return true; }

    //--------------------------------------------------------------------------
    void initialise (const juce::String& /*commandLine*/) override
    {
        // Create the main window — it owns the audio device manager and all UI
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr; // destructor will clean up audio device
    }

    void systemRequestedQuit() override { quit(); }

    //--------------------------------------------------------------------------
    // The main window wraps our MainComponent
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow (const juce::String& name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true); // MainComponent is our root UI
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() function for the platform
START_JUCE_APPLICATION (AdvancedTechnologiesApplication)
