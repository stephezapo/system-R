package org.stephezapo.system_r.core.ui;

import static org.stephezapo.system_r.core.ui.ColorTheme.COLOR_LIGHT_GRAY;
import static org.stephezapo.system_r.core.ui.ColorTheme.COLOR_MAIN_BACKGROUND;

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

    public void redraw()
    {
        getChildren().removeIf(node -> node instanceof Circle);

        this.setBackground(Background.fill(COLOR_MAIN_BACKGROUND));

        double width = this.getWidth();
        double height = this.getHeight();

        double pointsX = Math.round(width/POINT_SPACING);
        double pointsY = Math.round(height/POINT_SPACING);

        double actualSpacingX = width/pointsX;
        double actualSpacingY = height/pointsY;

        // Draw points in a regular square pattern
        for (double x = actualSpacingX; x < width; x += actualSpacingX)
        {
            for (double y = actualSpacingY; y < height; y += actualSpacingY)
            {
                Circle circle = new Circle(x, y, POINT_RADIUS, COLOR_LIGHT_GRAY);
                getChildren().add(circle);
            }
        }
    }
}
