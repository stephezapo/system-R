#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "ScreenPicker.h"
#include "ScreenWindow.h"

#include <array>
#include <functional>
#include <memory>

namespace systemr
{
/** Owns the screen windows: screen 1 (the main window, always open) and up to three
    extra screens (2-4) the user opens from the sidebar's Screens menu.

    Persists, per screen, whether it was open and its window state (position, size,
    maximised) in a PropertiesFile, and restores that layout on the next start. */
class ScreenManager final
{
public:
    static constexpr int numScreens = 4;

    /** `makeMainBody` builds screen 1's body (the main console view). Closing screen 1
        calls `onMainWindowClosed` (i.e. quits the app). */
    ScreenManager (juce::PropertiesFile& settings,
                   std::function<std::unique_ptr<juce::Component>()> makeMainBody,
                   std::function<void()> onMainWindowClosed)
        : props (settings), makeMain (std::move (makeMainBody)), quit (std::move (onMainWindowClosed))
    {
    }

    ~ScreenManager() { closeAll(); }

    /** Opens the main window plus every extra screen that was open at the last exit. */
    void restoreLayout()
    {
        open (1);
        for (int n = 2; n <= numScreens; ++n)
            if (props.getBoolValue (openKey (n)))
                open (n);
    }

    /** Records the open screens and their window states, then writes the settings file.
        Call before the windows are torn down (i.e. on app shutdown). */
    void saveLayout()
    {
        for (int n = 1; n <= numScreens; ++n)
        {
            if (auto& w = slot (n))
                props.setValue (stateKey (n), w->getWindowStateAsString());
            props.setValue (openKey (n), slot (n) != nullptr);
        }
        props.saveIfNeeded();
    }

    /** Destroys all windows (does not save). */
    void closeAll()
    {
        for (int n = numScreens; n >= 1; --n)
            slot (n) = nullptr;
    }

    bool isOpen (int n) const { return windows[static_cast<size_t> (n - 1)] != nullptr; }

    /** Opens screen `n`, or brings it to the front if it is already open. */
    void open (int n)
    {
        jassert (n >= 1 && n <= numScreens);
        auto& w = slot (n);

        if (w != nullptr)
        {
            w->setMinimised (false);
            w->toFront (true);
            return;
        }

        w = std::make_unique<ScreenWindow> (n, n == 1 ? makeMain()
                                                      : std::make_unique<EmptyScreenBody> (n));

        w->getSidebar().onItemClicked = [this] (Sidebar::Item item, juce::Component& source)
        {
            if (item == Sidebar::Item::screens)
                showScreensMenu (source);
        };

        w->onCloseRequested = [this, n]
        {
            if (n == 1)
            {
                quit();
                return;
            }
            // Defer: the window is still inside its own close-button callback.
            juce::MessageManager::callAsync ([this, n] { close (n); });
        };

        if (! w->restoreWindowStateFromString (props.getValue (stateKey (n))))
            placeByDefault (*w, n);

        w->setVisible (true);

        if (n != 1)
        {
            props.setValue (openKey (n), true);
            props.saveIfNeeded();
        }
    }

    /** Closes an extra screen (2-4), remembering where it was for the next time it opens. */
    void close (int n)
    {
        jassert (n >= 2 && n <= numScreens);
        auto& w = slot (n);
        if (w == nullptr)
            return;

        props.setValue (stateKey (n), w->getWindowStateAsString());
        props.setValue (openKey (n), false);
        props.saveIfNeeded();
        w = nullptr;
    }

    /** The sidebar's Screens popup: monitor icons for screens 2-4 in a row, opening to
        the right of `anchor` (the sidebar button). Clicking a screen toggles it: a closed
        screen opens, an open one closes. */
    void showScreensMenu (juce::Component& anchor)
    {
        juce::PopupMenu menu;
        menu.setLookAndFeel (&popupLookAndFeel);
        menu.addCustomItem (1, std::make_unique<ScreenPicker> (
                                   2, numScreens,
                                   [this] (int n) { return isOpen (n); },
                                   [this] (int n) { isOpen (n) ? close (n) : open (n); }),
                            nullptr, "Screens");

        // JUCE opens a popup *beside* its target only when the target area is empty, so pass
        // a zero-width strip on the sidebar's right edge, spanning the button's height.
        const auto button  = anchor.getScreenBounds();
        const auto rail    = anchor.getParentComponent() != nullptr
                                 ? anchor.getParentComponent()->getScreenBounds() : button;
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (
            { rail.getRight(), button.getY(), 0, button.getHeight() }));
    }

private:
    std::unique_ptr<ScreenWindow>& slot (int n) { return windows[static_cast<size_t> (n - 1)]; }

    static juce::String openKey (int n)  { return "screen" + juce::String (n) + ".open"; }
    static juce::String stateKey (int n) { return "screen" + juce::String (n) + ".windowState"; }

    /** First-run placement: main window centred, extra screens cascaded from it. */
    void placeByDefault (ScreenWindow& w, int n)
    {
        if (n == 1)
        {
            w.centreWithSize (w.getWidth(), w.getHeight());
            return;
        }

        auto origin = isOpen (1) ? slot (1)->getPosition()
                                 : juce::Desktop::getInstance().getDisplays()
                                       .getPrimaryDisplay()->userArea.getPosition();
        w.setTopLeftPosition (origin + juce::Point<int> (40, 40) * (n - 1));
    }

    juce::PropertiesFile& props;
    std::function<std::unique_ptr<juce::Component>()> makeMain;
    std::function<void()> quit;
    SidebarPopupLookAndFeel popupLookAndFeel;   // must outlive the windows' popups
    std::array<std::unique_ptr<ScreenWindow>, numScreens> windows;
};
} // namespace systemr
