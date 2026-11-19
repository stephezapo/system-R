#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Sidebar.h"

#include <functional>
#include <memory>

namespace systemr
{
/** What every screen window shows: the sidebar on the left, the screen's body on the right. */
class ScreenContent final : public juce::Component
{
public:
    explicit ScreenContent (std::unique_ptr<juce::Component> screenBody)
        : body (std::move (screenBody))
    {
        addAndMakeVisible (sidebar);
        addAndMakeVisible (*body);
        setSize (body->getWidth() + Sidebar::width, body->getHeight());
    }

    Sidebar& getSidebar() noexcept { return sidebar; }

    void resized() override
    {
        auto r = getLocalBounds();
        sidebar.setBounds (r.removeFromLeft (Sidebar::width));
        body->setBounds (r);
    }

private:
    Sidebar sidebar;
    std::unique_ptr<juce::Component> body;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScreenContent)
};

//==============================================================================
/** Placeholder body for screens 2-4 until their views are defined. */
class EmptyScreenBody final : public juce::Component
{
public:
    explicit EmptyScreenBody (int screenNumber) : number (screenNumber) { setSize (900, 560); }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour (0xff101010));
        g.setColour (juce::Colour (0xff505050));
        g.setFont (juce::FontOptions (22.0f));
        g.drawText ("Screen " + juce::String (number), getLocalBounds(),
                    juce::Justification::centred, false);
    }

private:
    int number;
};

//==============================================================================
/** A top-level, individually movable and resizable window showing one screen.
    Screen 1 is the main window; screens 2-4 are optional extra windows. */
class ScreenWindow final : public juce::DocumentWindow
{
public:
    ScreenWindow (int screenNumber, std::unique_ptr<juce::Component> body)
        : DocumentWindow (titleFor (screenNumber), juce::Colour (0xff101010),
                          DocumentWindow::allButtons),
          number (screenNumber)
    {
        setUsingNativeTitleBar (true);
        content = new ScreenContent (std::move (body));
        setContentOwned (content, true);
        setResizable (true, false);
        setResizeLimits (480, 320, 16384, 16384);
    }

    int getScreenNumber() const noexcept { return number; }
    Sidebar& getSidebar() noexcept       { return content->getSidebar(); }

    /** Invoked when the user clicks the window's close button. */
    std::function<void()> onCloseRequested;

    void closeButtonPressed() override
    {
        if (onCloseRequested)
            onCloseRequested();
    }

    static juce::String titleFor (int screenNumber)
    {
        return screenNumber == 1 ? juce::String ("system-R")
                                 : "system-R - Screen " + juce::String (screenNumber);
    }

private:
    int number;
    ScreenContent* content = nullptr;   // owned by the window

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScreenWindow)
};
} // namespace systemr
