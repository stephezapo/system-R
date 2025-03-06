package org.stephezapo.system_r.core.ui;

import javafx.application.Platform;
import org.stephezapo.system_r.core.Core;

public class UiCore
{
    public static void startup()
    {
        Core.Get().getAPI().startCore();
        openWindows();
    }

    public static void shutdown()
    {
        Platform.exit();
        System.exit(0);
    }

    private static void openWindows()
    {
        // Create window 1 (programmer)
        WindowManager.createMainWindow();

        // create window 2 (playback)
        //WindowManager.createPlaybackWindow();
        //WindowManager.createPlaybackWindow();

        // create window 3 (programmer panel)
        //WindowManager.createSpecialWindow();
    }
}
