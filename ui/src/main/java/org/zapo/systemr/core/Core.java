package org.zapo.systemr.core;

import javafx.application.Application;
import javafx.stage.Stage;
import org.zapo.lumosmaxima.remote.thrift.API;
import org.zapo.systemr.ui.window.ScreenManager;


public class Core {

    private static Core instance;

    private boolean init = false;
    private ScreenManager screens;
    private APIClient api;


    private Core() {

    }

    public static Core Get() {
        if(instance==null) {
            instance = new Core();
        }

        return instance;
    }

    public void init(Stage stage, Application appContext)
    {
        if(init)
        {
            return;
        }

        screens = new ScreenManager(stage, appContext);
        screens.showWindow(ScreenManager.WindowType.CONTROL);
        screens.showWindow(ScreenManager.WindowType.SCREEN1);
        api = new APIClient(2424);

        init = true;
    }

    public API.Client getAPI()
    {
        return api.getClient();
    }
}
