module org.szapo.systemrui {
    requires javafx.controls;
    requires javafx.fxml;
    requires kotlin.stdlib;


    opens org.szapo.systemrui to javafx.fxml;
    exports org.szapo.systemrui;
}