package org.stephezapo.system_r.core;

import atlantafx.base.theme.PrimerDark;
import javafx.application.Application;
import javafx.stage.Stage;
import org.stephezapo.system_r.core.ui.UiCore;
import org.stephezapo.system_r.core.ui.WindowManager;

public class Main
{
    public static final boolean DEVMODE = true;

    public static class MainApp extends Application
    {
        @Override
        public void start(Stage primaryStage)
        {
            Application.setUserAgentStylesheet(new PrimerDark().getUserAgentStylesheet());
            UiCore.startup();
        }
    }
    public static void main(String[] args)
    {
        Application.launch(MainApp.class);
    }
}