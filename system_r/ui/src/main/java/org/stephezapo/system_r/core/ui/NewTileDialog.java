package org.stephezapo.system_r.core.ui;

import javafx.geometry.HPos;
import javafx.geometry.Insets;
import javafx.geometry.VPos;
import javafx.scene.control.Button;
import javafx.scene.layout.ColumnConstraints;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.Priority;

public class NewTileDialog
    extends Dialog
{
    private final static int BUTTON_COLUMNS = 2;
    private GridPane gridPane = new GridPane();

    protected NewTileDialog(Window parent)
    {
        super(parent, "Create new Tile");

        for(int i = 0; i<BUTTON_COLUMNS; i++)
        {
            ColumnConstraints column = new ColumnConstraints();
            column.setPercentWidth(100.0/BUTTON_COLUMNS);
            column.setHalignment(HPos.CENTER);
            gridPane.getColumnConstraints().add(column);
        }

        gridPane.setVgap(20);
        gridPane.setHgap(10);
        gridPane.setLayoutY(getPanelRect().getMinY());
        gridPane.setPrefWidth(getPanelRect().getMaxX());
        gridPane.setPadding(new Insets(10, 10, 10, 10));
        getChildren().add(gridPane);

        Button dmxTileButton = new Button("DMX");
        dmxTileButton.setOnAction(actionEvent -> {
            parent.addTile(new DmxTile(parent));
            parent.closeDialog(this);
        });
        layoutButton(dmxTileButton, 0, 0);
    }

    private void layoutButton(Button button, int col, int row)
    {
        button.setMaxSize(Double.MAX_VALUE, Double.MAX_VALUE);

        GridPane.setHalignment(button, HPos.CENTER);
        GridPane.setHgrow(button, Priority.ALWAYS);
        GridPane.setValignment(button, VPos.CENTER);
        GridPane.setVgrow(button, Priority.ALWAYS);

        gridPane.add(button, col, row);
    }
}
