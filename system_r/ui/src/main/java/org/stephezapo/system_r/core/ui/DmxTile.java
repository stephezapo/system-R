package org.stephezapo.system_r.core.ui;

import static org.stephezapo.system_r.core.ui.Style.COLOR_TILE_TITLE;
import static org.stephezapo.system_r.core.ui.Style.DMX_GRID_COLUMN_COUNT;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import javafx.application.Platform;
import javafx.geometry.HPos;
import javafx.geometry.Rectangle2D;
import javafx.scene.control.Label;
import javafx.scene.layout.Background;
import javafx.scene.layout.ColumnConstraints;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.Priority;
import javafx.scene.paint.Color;
import org.stephezapo.system_r.core.Core;

public class DmxTile extends Tile
{
    private ScheduledExecutorService executorService;
    private ScheduledFuture<?> scheduledFuture;

    private GridPane gridPane = new GridPane();
    private Label[] dmxLabels = new Label[512];

    protected DmxTile(Window parent)
    {
        super(parent);
        setTitle("DMX");

        for(int i = 0; i< DMX_GRID_COLUMN_COUNT; i++)
        {
            ColumnConstraints column = new ColumnConstraints();
            column.setPercentWidth(100.0/DMX_GRID_COLUMN_COUNT);
            column.setFillWidth(true);
            column.setHgrow(Priority.ALWAYS) ;
            column.setHalignment(HPos.CENTER);
            gridPane.getColumnConstraints().add(column);
        }

        ColumnConstraints column2 = new ColumnConstraints();
        column2.setPercentWidth(50);

        getChildren().add(gridPane);

        for(int idx = 0; idx < 512; idx++)
        {
            dmxLabels[idx] = new Label();
            dmxLabels[idx].setTextFill(COLOR_TILE_TITLE);
            gridPane.add(dmxLabels[idx], idx % DMX_GRID_COLUMN_COUNT, (int)Math.floor(idx/(double)DMX_GRID_COLUMN_COUNT));
        }

        executorService = Executors.newScheduledThreadPool(1);
        scheduledFuture = executorService.scheduleAtFixedRate(this::updateDmx, 100, 50, TimeUnit.MILLISECONDS);
    }

    protected void moveAndScale(GridRect rect)
    {
        super.moveAndScale(rect);

        Rectangle2D bounds = getPanelRect();
        gridPane.setLayoutX(bounds.getMinX());
        gridPane.setLayoutY(bounds.getMinY());
        gridPane.setPrefWidth(bounds.getWidth());
        gridPane.setPrefHeight(bounds.getHeight());
    }

    private void updateDmx()
    {
        byte[] dmx = Core.getDmxUniverse();

        Platform.runLater(() -> {
            for(int i = 0; i<512; i++)
            {
                int value = dmx[i] & 0xff;
                dmxLabels[i].setText(String.valueOf(value));
                dmxLabels[i].setBackground(Background.fill(Color.rgb(0, value, 0)));
            }
        });
    }
}
