package org.stephezapo.system_r.core.ui;

import java.util.HashMap;
import java.util.Map;
import javafx.geometry.Point2D;
import org.stephezapo.system_r.core.ui.Window.WindowType;

public class WindowManager
{
    private static Map<WindowType, Window> windows = new HashMap<>();

    public static boolean createMainWindow()
    {
        if(windows.containsKey(WindowType.MAIN))
        {
            return false;
        }

        createWindow(WindowType.MAIN);
        return true;
    }

    public static boolean createPlaybackWindow()
    {
        if(!windows.containsKey(WindowType.PLAYBACK1))
        {
            createWindow(WindowType.PLAYBACK1);
            return true;
        }

        if(!windows.containsKey(WindowType.PLAYBACK2))
        {
            createWindow(WindowType.PLAYBACK2);
            return true;
        }

        return false;
    }

    public static boolean createSpecialWindow()
    {
        if(!windows.containsKey(WindowType.SPECIAL1))
        {
            createWindow(WindowType.SPECIAL1);
            return true;
        }

        if(!windows.containsKey(WindowType.SPECIAL2))
        {
            createWindow(WindowType.SPECIAL2);
            return true;
        }

        return false;
    }

    public static boolean createExternalWindow()
    {
        if(!windows.containsKey(WindowType.EXTERNAL1))
        {
            createWindow(WindowType.EXTERNAL1);
            return true;
        }

        if(!windows.containsKey(WindowType.EXTERNAL2))
        {
            createWindow(WindowType.EXTERNAL2);
            return true;
        }

        return false;
    }

    protected static void closeWindow(WindowType type)
    {
        windows.remove(type);

        if(windows.isEmpty())
        {
            UiCore.shutdown();
        }
    }

    private static void createWindow(WindowType type)
    {
        Window window = new Window(type);
        windows.put(type, window);
        window.show();
    }

    protected GridPoint getWindowGridSize(WindowType type)
    {
        if(windows.containsKey(type))
        {
            return windows.get(type).getWindowGridSize();
        }

        return null;
    }
}
