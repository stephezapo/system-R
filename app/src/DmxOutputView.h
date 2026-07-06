#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "system_r/core/Engine.h"
#include "system_r/core/OutputConfig.h"
#include "system_r/core/TripleBuffer.h"
#include "system_r/core/Universe.h"
#include "system_r/core/artnet/ArtNetPacket.h"
#include "system_r/core/sacn/SacnSender.h"

#include <utility>
#include <vector>

namespace systemr
{
/** Live DMX Output view: a 32×16 grid of the 512 slots of one universe, polled from
    the engine's output snapshot and refreshed at ~30 Hz. A selector switches which
    configured output universe is shown. */
class DmxOutputView final : public juce::Component,
                            private juce::Timer
{
public:
    DmxOutputView (TripleBuffer<UniverseSet>& out, std::vector<UniverseOutput> cfgs)
        : output (out), configs (std::move (cfgs))
    {
        universeSelect.setJustificationType (juce::Justification::centredLeft);
        for (std::size_t i = 0; i < configs.size(); ++i)
            universeSelect.addItem (describe (configs[i]), static_cast<int> (i) + 1);
        universeSelect.setSelectedItemIndex (0, juce::dontSendNotification);
        universeSelect.onChange = [this]
        {
            selected = juce::jmax (0, universeSelect.getSelectedItemIndex());
            repaint();
        };
        addAndMakeVisible (universeSelect);

        caption.setText ("DMX Output", juce::dontSendNotification);
        caption.setFont (juce::FontOptions (16.0f, juce::Font::bold));
        addAndMakeVisible (caption);

        startTimerHz (30);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        auto top = r.removeFromTop (28);
        caption.setBounds (top.removeFromLeft (110));
        universeSelect.setBounds (top.removeFromLeft (240));
        r.removeFromTop (6);
        grid = r;
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff141414));

        if (configs.empty())
        {
            g.setColour (juce::Colours::grey);
            g.drawText ("No output universes configured", getLocalBounds(),
                        juce::Justification::centred);
            return;
        }

        constexpr int cols = 32;
        constexpr int rows = 16;
        const float cw = grid.getWidth()  / static_cast<float> (cols);
        const float ch = grid.getHeight() / static_cast<float> (rows);
        const bool showValues = cw >= 24.0f && ch >= 15.0f;

        g.setFont (juce::FontOptions (juce::jmin (12.0f, ch - 3.0f)));

        for (int i = 0; i < kUniverseSize; ++i)
        {
            const int col = i % cols;
            const int row = i / cols;
            const auto cell = juce::Rectangle<float> (grid.getX() + col * cw,
                                                      grid.getY() + row * ch,
                                                      cw, ch).reduced (0.5f);

            const auto v = displayed[static_cast<std::size_t> (i)];
            g.setColour (cellColour (v));
            g.fillRect (cell);

            if (showValues && v > 0)
            {
                g.setColour (v > 140 ? juce::Colours::black : juce::Colours::white);
                g.drawText (juce::String (v), cell, juce::Justification::centred);
            }
        }
    }

private:
    void timerCallback() override
    {
        if (output.acquire())
            latched = true;

        if (latched)
        {
            const auto& frames = output.readSlot();
            if (selected < static_cast<int> (frames.size()))
            {
                displayed = frames[static_cast<std::size_t> (selected)];
                repaint (grid);
            }
        }
    }

    static juce::Colour cellColour (std::uint8_t v)
    {
        if (v == 0)
            return juce::Colour (0xff202020);
        const float t = v / 255.0f;
        return juce::Colour::fromHSV (0.55f, 0.18f, 0.22f + 0.78f * t, 1.0f);
    }

    static juce::String describe (const UniverseOutput& cfg)
    {
        juce::String s = "Universe " + juce::String (cfg.index);
        juce::StringArray wire;
        if (cfg.usesSacn())
            wire.add ("sACN " + juce::String (cfg.sacnUniverse));
        if (cfg.usesArtNet())
            wire.add ("Art-Net " + artnet::formatPortAddress (cfg.artnetPortAddress));
        if (! wire.isEmpty())
            s << "  (" << wire.joinIntoString (" + ") << ")";
        return s;
    }

    TripleBuffer<UniverseSet>& output;
    std::vector<UniverseOutput> configs;
    Universe displayed {};
    bool latched = false;
    int selected = 0;

    juce::Rectangle<int> grid;
    juce::Label caption;
    juce::ComboBox universeSelect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DmxOutputView)
};
}
