import javafx.application.Application;
import javafx.stage.Stage;
import org.zapo.systemr.core.Core;

public class Main extends Application {


    @Override
    public void start(Stage stage) throws Exception {

        Core.Get().init(stage, this); //initialize UI
        /*Parent root = FXMLLoader.load(getClass().getResource("MainWindow.fxml"));
        Scene scene = new Scene(root);
        stage.setScene(scene);
        stage.show();*/
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        launch(args);
    }
}
