package org.stephezapo.system_r.core.ui;

import javafx.scene.Scene;
import javafx.scene.layout.ColumnConstraints;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.RowConstraints;

public class ControlsWindow extends Window
{
    private static final int WINDOW_WIDTH = 1600;
    private static final int WINDOW_HEIGHT = 500;
    private static final int ROWS = 10;
    private static final int COLUMNS = 19;

    public ControlsWindow()
    {
        super(WindowType.CONTROLS);

        GridPane gridPane = new GridPane();

        // Calculate cell sizes
        double cellWidth = (double) WINDOW_WIDTH / COLUMNS;
        double cellHeight = (double) WINDOW_HEIGHT / ROWS;

        // Define column constraints
        for (int col = 0; col < COLUMNS; col++) {
            ColumnConstraints column = new ColumnConstraints();
            column.setPrefWidth(cellWidth);
            gridPane.getColumnConstraints().add(column);
        }

        // Define row constraints
        for (int row = 0; row < ROWS; row++) {
            RowConstraints rowConstraint = new RowConstraints();
            rowConstraint.setPrefHeight(cellHeight);
            gridPane.getRowConstraints().add(rowConstraint);
        }

        Scene scene = new Scene(gridPane, WINDOW_WIDTH, WINDOW_HEIGHT);
        setScene(scene);
    }
}
