package org.stephezapo.system_r.core.ui;

import javafx.application.Platform;

public class UiCore
{
    public static void startup()
    {
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
