#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Icons.h"

#include <functional>
#include <memory>
#include <vector>

namespace systemr
{
/** Content of the sidebar's Screens popup: a horizontal row of monitor icons, each with
    its screen number drawn inside. Open screens are shown in the accent colour.
    Clicking an icon dismisses the popup and reports the screen number. */
class ScreenPicker final : public juce::PopupMenu::CustomComponent
{
public:
    static constexpr int itemSize = 56;
    static constexpr int gap      = 4;

    /** `firstScreen`..`lastScreen` are shown left to right; `isOpen` drives the highlight;
        `onPick` is invoked asynchronously, after the popup has closed. */
    ScreenPicker (int firstScreen, int lastScreen,
                  std::function<bool (int)> isOpen, std::function<void (int)> onPick)
        : CustomComponent (false)   // children handle the clicks
    {
        for (int n = firstScreen; n <= lastScreen; ++n)
        {
            auto& b = buttons.emplace_back (std::make_unique<ScreenButton> (n, isOpen (n)));
            b->setTooltip ((isOpen (n) ? "Close screen " : "Open screen ") + juce::String (n));
            b->onClick = [this, n, onPick]
            {
                juce::MessageManager::callAsync ([onPick, n] { onPick (n); });
                triggerMenuItem();   // closes the popup; must be the last thing we touch
            };
            addAndMakeVisible (*b);
        }
    }

    void getIdealSize (int& w, int& h) override
    {
        const auto n = static_cast<int> (buttons.size());
        w = n * itemSize + (n - 1) * gap;
        h = itemSize;
    }

    void resized() override
    {
        auto r = getLocalBounds();
        for (auto& b : buttons)
        {
            b->setBounds (r.removeFromLeft (itemSize));
            r.removeFromLeft (gap);
        }
    }

private:
    class ScreenButton final : public juce::Button
    {
    public:
        ScreenButton (int screenNumber, bool screenIsOpen)
            : juce::Button ("Screen " + juce::String (screenNumber)),
              number (screenNumber), open (screenIsOpen)
        {
        }

        void paintButton (juce::Graphics& g, bool highlighted, bool down) override
        {
            const auto area = getLocalBounds().toFloat();

            if (highlighted || down)
            {
                g.setColour (juce::Colours::white.withAlpha (down ? 0.16f : 0.08f));
                g.fillRoundedRectangle (area, 8.0f);
            }

            const auto colour = open        ? juce::Colour (0xff42a2c8)
                              : highlighted ? juce::Colours::white
                                            : juce::Colour (0xffa0a0a0);
            g.setColour (colour);

            // Monitor glyph, fitted by its ink bounds so the screen area is known exactly.
            auto path = icons::outline (icons::Glyph::monitor);
            const auto iconBox = area.withSizeKeepingCentre (area.getWidth() * 0.62f,
                                                             area.getHeight() * 0.62f);
            path.applyTransform (path.getTransformToScaleToFit (iconBox, true));
            g.fillPath (path);

            // Lucide "monitor": ink spans y 2..22 of the 24 grid; the screen's inside is
            // y 4..16 (centre 10), i.e. 40% down the ink height and 60% of it tall.
            const auto ink = path.getBounds();
            const auto screenCentre = juce::Point<float> (ink.getCentreX(),
                                                          ink.getY() + ink.getHeight() * 0.4f);
            const auto textH = ink.getHeight() * 0.5f;
            g.setFont (juce::FontOptions (textH, juce::Font::bold));
            g.drawText (juce::String (number),
                        juce::Rectangle<float> (ink.getWidth(), textH).withCentre (screenCentre),
                        juce::Justification::centred, false);
        }

    private:
        int number;
        bool open;
    };

    std::vector<std::unique_ptr<ScreenButton>> buttons;
};

//==============================================================================
/** Dark styling for the sidebar's popups, matching the sidebar. */
struct SidebarPopupLookAndFeel final : juce::LookAndFeel_V4
{
    SidebarPopupLookAndFeel()
    {
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff202020));
    }

    void drawPopupMenuBackgroundWithOptions (juce::Graphics& g, int w, int h,
                                             const juce::PopupMenu::Options&) override
    {
        g.fillAll (findColour (juce::PopupMenu::backgroundColourId));
        g.setColour (juce::Colour (0xff3a3a3a));
        g.drawRect (0, 0, w, h, 1);
    }

    int getPopupMenuBorderSizeWithOptions (const juce::PopupMenu::Options&) override { return 6; }
};
} // namespace systemr
