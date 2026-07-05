#include <juce_gui_extra/juce_gui_extra.h>

#include "system_r/core/Engine.h"
#include "system_r/core/Programmer.h"
#include "system_r/core/ShowDocument.h"
#include "system_r/core/TripleBuffer.h"
#include "system_r/core/Universe.h"
#include "system_r/core/sacn/SacnSender.h"

namespace systemr
{
//==============================================================================
/** Phase 1 walking-skeleton UI: one dimmer fader drives the programmer; a live
    readout polls the engine's output snapshot. Proves UI → engine → sACN wire. */
class MainComponent final : public juce::Component,
                            private juce::Timer
{
public:
    MainComponent (Programmer& p, TripleBuffer<Universe>& out, juce::String target)
        : programmer (p), output (out), sacnTarget (std::move (target))
    {
        title.setText ("system-R", juce::dontSendNotification);
        title.setFont (juce::FontOptions (28.0f, juce::Font::bold));
        title.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (title);

        status.setText ("sACN out  ->  " + sacnTarget, juce::dontSendNotification);
        status.setColour (juce::Label::textColourId, juce::Colours::grey);
        addAndMakeVisible (status);

        dimmer.setSliderStyle (juce::Slider::LinearVertical);
        dimmer.setRange (0.0, 255.0, 1.0);
        dimmer.setValue (0.0, juce::dontSendNotification);
        dimmer.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 24);
        dimmer.onValueChange = [this]
        {
            programmer.setDimmer (static_cast<std::uint8_t> (dimmer.getValue()));
        };
        addAndMakeVisible (dimmer);

        dimmerCaption.setText ("Dimmer 1", juce::dontSendNotification);
        dimmerCaption.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (dimmerCaption);

        liveReadout.setFont (juce::FontOptions (16.0f));
        liveReadout.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (liveReadout);

        setSize (420, 560);
        startTimerHz (30);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff141414));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (20);
        title.setBounds (r.removeFromTop (40));
        status.setBounds (r.removeFromTop (24));
        r.removeFromTop (16);
        liveReadout.setBounds (r.removeFromBottom (28));
        dimmerCaption.setBounds (r.removeFromBottom (24));
        dimmer.setBounds (r.withSizeKeepingCentre (120, r.getHeight()));
    }

private:
    void timerCallback() override
    {
        if (output.acquire())
            liveValue = output.readSlot()[0];

        liveReadout.setText ("live DMX  U1 / addr 1  =  " + juce::String (liveValue),
                             juce::dontSendNotification);
    }

    Programmer& programmer;
    TripleBuffer<Universe>& output;
    juce::String sacnTarget;
    int liveValue = 0;

    juce::Label title, status, dimmerCaption, liveReadout;
    juce::Slider dimmer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

//==============================================================================
class MainWindow final : public juce::DocumentWindow
{
public:
    MainWindow (const juce::String& name, Programmer& programmer,
                TripleBuffer<Universe>& output, const juce::String& sacnTarget)
        : DocumentWindow (name,
                          juce::Desktop::getInstance().getDefaultLookAndFeel()
                              .findColour (juce::ResizableWindow::backgroundColourId),
                          DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar (true);
        setContentOwned (new MainComponent (programmer, output, sacnTarget), true);
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

//==============================================================================
class SystemRApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "system-R"; }
    const juce::String getApplicationVersion() override { return "0.0.1"; }
    bool moreThanOneInstanceAllowed() override          { return true; }

    void initialise (const juce::String&) override
    {
        sender.setUniverse (1);
        sender.open();

        engine = std::make_unique<Engine> (show, programmer, sender);
        engine->start (40);

        mainWindow = std::make_unique<MainWindow> (getApplicationName(), programmer,
                                                   engine->output(), sender.multicastAddress());
    }

    void shutdown() override
    {
        mainWindow = nullptr;   // tears down the UI before the engine it observes
        if (engine != nullptr)
            engine->stop();
        engine = nullptr;
    }

    void systemRequestedQuit() override { quit(); }

private:
    ShowDocument show;
    Programmer programmer;
    SacnSender sender;
    std::unique_ptr<Engine> engine;
    std::unique_ptr<MainWindow> mainWindow;
};
} // namespace systemr

//==============================================================================
START_JUCE_APPLICATION (systemr::SystemRApplication)
