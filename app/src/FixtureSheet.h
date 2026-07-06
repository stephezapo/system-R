#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "system_r/core/Engine.h"
#include "system_r/core/OutputConfig.h"
#include "system_r/core/ShowDocument.h"
#include "system_r/core/TripleBuffer.h"
#include "system_r/core/Universe.h"

#include <utility>
#include <vector>

namespace systemr
{
/** Fixture Sheet: one row per patched fixture showing its live DMX values (the channels it
    occupies), polled from the engine snapshot at ~20 Hz. Fixtures on universes that aren't
    an output universe show no live values. */
class FixtureSheet final : public juce::Component,
                           private juce::Timer
{
public:
    FixtureSheet (ShowDocument& s, std::vector<UniverseOutput> cfgs, TripleBuffer<UniverseSet>& out)
        : show (s), configs (std::move (cfgs)), output (out)
    {
        startTimerHz (20);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff141414));

        const auto fixtures = show.fixtures();
        auto area = getLocalBounds().reduced (8);

        g.setColour (juce::Colours::grey);
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.drawText ("Fixture", area.getX(), area.getY(), 220, 20, juce::Justification::centredLeft);
        g.drawText ("Live values", area.getX() + 230, area.getY(), 200, 20, juce::Justification::centredLeft);
        area.removeFromTop (24);

        constexpr int rowH = 22;
        for (const auto& f : fixtures)
        {
            if (area.getHeight() < rowH)
                break;
            auto row = area.removeFromTop (rowH);
            area.removeFromTop (2);

            g.setColour (juce::Colours::white);
            g.setFont (juce::FontOptions (13.0f));
            const auto label = juce::String (f.fid) + "  " + f.name
                                 + "   U" + juce::String (f.universe) + "." + juce::String (f.address);
            g.drawText (label, row.removeFromLeft (220), juce::Justification::centredLeft);

            row.removeFromLeft (10);
            paintValues (g, row, f);
        }
    }

private:
    void timerCallback() override
    {
        if (output.acquire())
            latched = true;
        if (latched)
        {
            frames = output.readSlot();
            repaint();
        }
    }

    int frameIndexForUniverse (int universe) const
    {
        for (std::size_t i = 0; i < configs.size(); ++i)
            if (configs[i].index == universe)
                return static_cast<int> (i);
        return -1;
    }

    void paintValues (juce::Graphics& g, juce::Rectangle<int> row, const ShowDocument::Fixture& f)
    {
        const int frameIdx = frameIndexForUniverse (f.universe);
        if (frameIdx < 0 || frameIdx >= (int) frames.size())
        {
            g.setColour (juce::Colours::darkgrey);
            g.drawText ("(not an output universe)", row, juce::Justification::centredLeft);
            return;
        }

        const auto& frame = frames[static_cast<std::size_t> (frameIdx)];
        const int cellW = 30;
        const int maxCells = juce::jmin (f.channelCount, row.getWidth() / cellW);

        for (int c = 0; c < maxCells; ++c)
        {
            const int slot = f.address - 1 + c; // 0-based
            if (slot < 0 || slot >= kUniverseSize)
                break;
            const auto v = frame[static_cast<std::size_t> (slot)];

            auto cell = row.removeFromLeft (cellW).reduced (1);
            g.setColour (v == 0 ? juce::Colour (0xff202020)
                                : juce::Colour::fromHSV (0.55f, 0.18f, 0.25f + 0.75f * (v / 255.0f), 1.0f));
            g.fillRect (cell);
            g.setColour (v > 140 ? juce::Colours::black : juce::Colours::lightgrey);
            g.setFont (juce::FontOptions (11.0f));
            g.drawText (juce::String (v), cell, juce::Justification::centred);
        }
    }

    ShowDocument& show;
    std::vector<UniverseOutput> configs;
    TripleBuffer<UniverseSet>& output;
    UniverseSet frames;
    bool latched = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FixtureSheet)
};
}
