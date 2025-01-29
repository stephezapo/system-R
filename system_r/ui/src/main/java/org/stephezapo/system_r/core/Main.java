package org.stephezapo.system_r.core;

import javafx.application.Application;
import javafx.stage.Stage;
import org.stephezapo.system_r.core.ui.WindowManager;

public class Main
{
    public static class MainApp extends Application
    {
        @Override
        public void start(Stage primaryStage)
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
    public static void main(String[] args)
    {
        Application.launch(MainApp.class);
    }
}