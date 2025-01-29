package org.stephezapo.system_r.core;

import javafx.application.Application;
import javafx.stage.Stage;
import org.stephezapo.system_r.core.ui.UiCore;
import org.stephezapo.system_r.core.ui.WindowManager;

public class Main
{
    public static class MainApp extends Application
    {
        @Override
        public void start(Stage primaryStage)
        {
            UiCore.startup();
        }
    }
    public static void main(String[] args)
    {
        Application.launch(MainApp.class);
    }
}