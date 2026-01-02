package org.stephezapo.system_r.core.ui;

import javafx.application.Platform;
import org.stephezapo.system_r.core.Core;
import org.stephezapo.system_r.core.Main;

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
        WindowManager.createProgrammerWindow();
        WindowManager.createPlaybackWindow();
    }
}
