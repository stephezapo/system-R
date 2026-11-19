#include <juce_gui_extra/juce_gui_extra.h>

#include "DmxOutputView.h"
#include "FixtureSheet.h"
#include "PatchPanel.h"
#include "Sidebar.h"

#include "system_r/core/Engine.h"
#include "system_r/core/OutputConfig.h"
#include "system_r/core/OutputManager.h"
#include "system_r/core/Programmer.h"
#include "system_r/core/ShowDocument.h"
#include "system_r/core/TripleBuffer.h"
#include "system_r/core/Universe.h"
#include "system_r/core/artnet/ArtNetPacket.h"
#include "system_r/core/artnet/ArtNetReceiver.h"
#include "system_r/core/gdtf/FixtureLibrary.h"
#include "system_r/core/net/InputMerger.h"
#include "system_r/core/sacn/SacnReceiver.h"
#include "system_r/core/sacn/SacnSender.h"

#include <vector>

namespace systemr
{
//==============================================================================
/** Live tab: one dimmer fader drives the programmer; a live readout plus the DMX output
    grid poll the engine's output snapshot. Proves UI → engine → sACN wire. */
class LiveView final : public juce::Component,
                       private juce::Timer
{
public:
    LiveView (Programmer& p, TripleBuffer<UniverseSet>& out,
              std::vector<UniverseOutput> configs, juce::String target)
        : programmer (p), output (out), sacnTarget (std::move (target)),
          dmxView (out, configs)
    {
        title.setText ("system-R", juce::dontSendNotification);
        title.setFont (juce::FontOptions (28.0f, juce::Font::bold));
        title.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (title);

        status.setText ("out  ->  " + sacnTarget, juce::dontSendNotification);
        status.setColour (juce::Label::textColourId, juce::Colours::grey);
        status.setJustificationType (juce::Justification::topLeft);
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

        liveReadout.setFont (juce::FontOptions (14.0f));
        liveReadout.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (liveReadout);

        addAndMakeVisible (dmxView);

        setSize (980, 600);
        startTimerHz (30);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff101010));
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto left = r.removeFromLeft (220).reduced (16);

        title.setBounds (left.removeFromTop (40));
        status.setBounds (left.removeFromTop (40));
        left.removeFromTop (8);
        liveReadout.setBounds (left.removeFromBottom (26));
        dimmerCaption.setBounds (left.removeFromBottom (22));
        dimmer.setBounds (left.withSizeKeepingCentre (110, left.getHeight()));

        dmxView.setBounds (r);
    }

private:
    void timerCallback() override
    {
        if (output.acquire())
            latched = true;

        if (latched)
        {
            const auto& frames = output.readSlot();
            if (! frames.empty())
                liveValue = frames[0][0];
        }

        liveReadout.setText ("U1 / addr 1  =  " + juce::String (liveValue),
                             juce::dontSendNotification);
    }

    Programmer& programmer;
    TripleBuffer<UniverseSet>& output;
    juce::String sacnTarget;
    int liveValue = 0;
    bool latched = false;

    juce::Label title, status, dimmerCaption, liveReadout;
    juce::Slider dimmer;
    DmxOutputView dmxView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveView)
};

//==============================================================================
/** Root content: the icon sidebar on the left, and a tabbed shell over the Live view, the
    Patch panel and the Fixture Sheet. (The full tiled multi-window layout is Phase 6.) */
class RootComponent final : public juce::Component
{
public:
    RootComponent (Programmer& programmer, TripleBuffer<UniverseSet>& output,
                   std::vector<UniverseOutput> configs, ShowDocument& show,
                   gdtf::FixtureLibrary& library, Engine& engine, juce::String target)
        : liveView (programmer, output, configs, std::move (target)),
          patchPanel (show, library, engine),
          fixtureSheet (show, configs, output),
          tabs (juce::TabbedButtonBar::TabsAtTop)
    {
        const auto bg = juce::Colour (0xff101010);
        tabs.addTab ("Live", bg, &liveView, false);
        tabs.addTab ("Patch", bg, &patchPanel, false);
        tabs.addTab ("Fixtures", bg, &fixtureSheet, false);
        addAndMakeVisible (tabs);

        // Menu actions (File / Screens / Settings) are wired up later.
        addAndMakeVisible (sidebar);

        setSize (980 + Sidebar::width, 620);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        sidebar.setBounds (r.removeFromLeft (Sidebar::width));
        tabs.setBounds (r);
    }

private:
    LiveView liveView;
    PatchPanel patchPanel;
    FixtureSheet fixtureSheet;
    juce::TabbedComponent tabs;
    Sidebar sidebar;
    juce::TooltipWindow tooltips { this, 600 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RootComponent)
};

//==============================================================================
class MainWindow final : public juce::DocumentWindow
{
public:
    MainWindow (const juce::String& name, Programmer& programmer,
                TripleBuffer<UniverseSet>& output, std::vector<UniverseOutput> configs,
                ShowDocument& show, gdtf::FixtureLibrary& library, Engine& engine,
                const juce::String& sacnTarget)
        : DocumentWindow (name,
                          juce::Desktop::getInstance().getDefaultLookAndFeel()
                              .findColour (juce::ResizableWindow::backgroundColourId),
                          DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar (true);
        setContentOwned (new RootComponent (programmer, output, std::move (configs),
                                            show, library, engine, sacnTarget), true);
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
        // Default output: four sACN universes on their Appendix-A multicast groups.
        std::vector<UniverseOutput> configs;
        std::vector<int> sacnUniverses;
        for (int u = 1; u <= 4; ++u)
        {
            UniverseOutput cfg;
            cfg.index             = u;
            cfg.protocol          = OutputProtocol::sacn;
            cfg.sacnUniverse      = static_cast<std::uint16_t> (u);
            cfg.artnetPortAddress = artnet::makePortAddress (0, 0, u - 1);
            configs.push_back (cfg);
            sacnUniverses.push_back (u);
        }

        output.setConfig (configs);
        output.open();

        engine = std::make_unique<Engine> (show, library, programmer, output);
        engine->start (40);

        // Input: listen for external sACN/Art-Net and merge it (monitor + tracking-backup).
        sacnReceiver.start (sacnUniverses);

        artnet::PollReplyConfig reply;
        const auto local = juce::IPAddress::getLocalAddress();
        for (int i = 0; i < 4; ++i)
            reply.ip[static_cast<std::size_t> (i)] = static_cast<std::uint8_t> (local.address[i]);
        reply.numPorts = juce::jmin (4, static_cast<int> (configs.size()));
        for (int i = 0; i < reply.numPorts; ++i)
            reply.swOut[static_cast<std::size_t> (i)] =
                static_cast<std::uint16_t> (configs[static_cast<std::size_t> (i)].artnetPortAddress & 0x0f);
        artnetReceiver.setPollReply (reply);
        artnetReceiver.start();

        const auto target = SacnSender::multicastAddress (1) + "  (+3 universes)";
        mainWindow = std::make_unique<MainWindow> (getApplicationName(), programmer,
                                                   engine->output(), configs,
                                                   show, library, *engine, target);
    }

    void shutdown() override
    {
        mainWindow = nullptr;   // tears down the UI before the engine it observes
        sacnReceiver.stop();
        artnetReceiver.stop();
        if (engine != nullptr)
            engine->stop();
        engine = nullptr;
    }

    void systemRequestedQuit() override { quit(); }

private:
    ShowDocument show;
    gdtf::FixtureLibrary library;
    Programmer programmer;
    OutputManager output;
    InputMerger inputMerger;
    SacnReceiver sacnReceiver { inputMerger };
    ArtNetReceiver artnetReceiver { inputMerger };
    std::unique_ptr<Engine> engine;
    std::unique_ptr<MainWindow> mainWindow;
};
} // namespace systemr

//==============================================================================
START_JUCE_APPLICATION (systemr::SystemRApplication)
