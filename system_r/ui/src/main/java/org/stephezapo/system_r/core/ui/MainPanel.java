package org.stephezapo.system_r.core.ui;

import static org.stephezapo.system_r.core.ui.ColorTheme.COLOR_MAIN_BACKGROUND;
import static org.stephezapo.system_r.core.ui.ColorTheme.COLOR_TILE_HEADER;

import javafx.scene.layout.Background;
import javafx.scene.layout.Pane;
import javafx.scene.shape.Circle;

public class MainPanel extends Pane
{
    private static final int POINT_SPACING = 50;
    private static final double POINT_RADIUS = 0.5;

    public MainPanel()
    {

    }

    public void paint()
    {
        this.setBackground(Background.fill(COLOR_MAIN_BACKGROUND));
        double width = this.getWidth();
        double height = this.getHeight();

        // Draw points in a regular square pattern
        for (int x = 0; x < width; x += POINT_SPACING)
        {
            for (int y = 0; y < height; y += POINT_SPACING)
            {
                Circle circle = new Circle(x, y, POINT_RADIUS, COLOR_TILE_HEADER);
                getChildren().add(circle);
            }
        }

    }
}
