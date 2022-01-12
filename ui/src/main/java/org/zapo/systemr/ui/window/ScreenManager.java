package org.zapo.systemr.ui.window;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;

public class ScreenManager {

    public enum WindowType {
        CONTROL,
        SCREEN1,
        SCREEN2,
        SCREEN3
    };

    private Stage stage;
    private Application appContext;


    public ScreenManager(Stage stage, Application appContext) {
        this.stage = stage;
        this.appContext = appContext;
    }

    public void showWindow(WindowType type) {

        try {
            String fxml;

            switch(type) {
                case CONTROL:
                    fxml = "ControlPanel.fxml";
                    break;
                default:
                    fxml = "MainWindow.fxml";
                    break;
            }

            Parent root = FXMLLoader.load(appContext.getClass().getResource(fxml));
            Stage newStage = new Stage();
            newStage.initOwner(stage);

            Scene scene = new Scene(root);
            newStage.setScene(scene);
            newStage.show();
        }
        catch(Exception ex) {
            ex.printStackTrace();
        }
    }
}
