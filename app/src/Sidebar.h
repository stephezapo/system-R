#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Icons.h"

#include <array>
#include <functional>
#include <memory>
#include <vector>

namespace systemr
{
/** Left-hand navigation rail: a column of icon buttons (Lucide glyphs) with tooltips.
    What each item does is wired up later through `onItemClicked`. */
class Sidebar final : public juce::Component
{
public:
    enum class Item { file, screens, settings };

    static constexpr int width = 64;

    /** Called when the user clicks a menu item. */
    std::function<void (Item)> onItemClicked;

    Sidebar()
    {
        for (const auto& def : itemDefs)
        {
            auto& b = buttons.emplace_back (std::make_unique<IconButton> (def.name, def.glyph));
            b->setTooltip (def.name);
            b->onClick = [this, item = def.item]
            {
                if (onItemClicked)
                    onItemClicked (item);
            };
            addAndMakeVisible (*b);
        }
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff181818));
        g.setColour (juce::Colour (0xff2a2a2a));   // divider against the content area
        g.fillRect (getLocalBounds().removeFromRight (1));
    }

    void resized() override
    {
        auto r = getLocalBounds().withTrimmedRight (1).reduced (8);
        for (auto& b : buttons)
        {
            b->setBounds (r.removeFromTop (r.getWidth()));
            r.removeFromTop (8);
        }
    }

private:
    /** A square button that paints a single Lucide glyph, with hover/press feedback. */
    class IconButton final : public juce::Button
    {
    public:
        IconButton (const juce::String& name, icons::Glyph g) : juce::Button (name), glyph (g) {}

        void paintButton (juce::Graphics& g, bool highlighted, bool down) override
        {
            const auto area = getLocalBounds().toFloat();

            if (highlighted || down)
            {
                g.setColour (juce::Colours::white.withAlpha (down ? 0.16f : 0.08f));
                g.fillRoundedRectangle (area, 8.0f);
            }

            g.setColour (highlighted || down ? juce::Colours::white : juce::Colour (0xffa0a0a0));
            icons::draw (g, glyph, area, area.getHeight() * 0.5f);
        }

    private:
        icons::Glyph glyph;
    };

    struct ItemDef
    {
        Item item;
        const char* name;
        icons::Glyph glyph;
    };

    static constexpr std::array<ItemDef, 3> itemDefs {{
        { Item::file,     "File",     icons::Glyph::folder },
        { Item::screens,  "Screens",  icons::Glyph::monitor },
        { Item::settings, "Settings", icons::Glyph::settings },
    }};

    std::vector<std::unique_ptr<IconButton>> buttons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sidebar)
};
} // namespace systemr
