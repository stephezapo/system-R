package org.stephezapo.system_r.core.ui;

import static org.stephezapo.system_r.core.ui.ColorTheme.COLOR_LIGHT_GRAY;
import static org.stephezapo.system_r.core.ui.ColorTheme.COLOR_MAIN_BACKGROUND;

import javafx.geometry.Point2D;
import javafx.scene.DepthTest;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Background;
import javafx.scene.layout.Pane;
import javafx.scene.shape.Circle;

public class GridPanel
    extends Pane
{
    private static final int POINT_SPACING = 50;
    private static final double POINT_RADIUS = 0.5;
    private Point2D gridSize = new Point2D(1, 1);
    private Point2D cellSize = new Point2D(1, 1);
    private Window parentWindow;

    public GridPanel(Window window)
    {
        parentWindow = window;
        addEventFilter(MouseEvent.MOUSE_CLICKED, e -> mouseClick(e.getSceneX(), e.getSceneY()));
    }

    public void redraw()
    {
        getChildren().removeIf(node -> node instanceof Circle);

        this.setBackground(Background.fill(COLOR_MAIN_BACKGROUND));

        double width = this.getWidth();
        double height = this.getHeight();

        gridSize = new Point2D(Math.round(width/POINT_SPACING), Math.round(height/POINT_SPACING));

        cellSize = new Point2D(width/gridSize.getX(), height/gridSize.getY());

        // Draw points in a regular square pattern
        for (double x = cellSize.getX(); x < width; x += cellSize.getX())
        {
            for (double y = cellSize.getY(); y < height; y += cellSize.getY())
            {
                Circle circle = new Circle(x, y, POINT_RADIUS, COLOR_LIGHT_GRAY);
                circle.setViewOrder(Double.MAX_VALUE);
                getChildren().add(circle);
            }
        }
    }

    protected Point2D getGridSize()
    {
        return gridSize;
    }

    protected Point2D getGridCellSize()
    {
        return cellSize;
    }

    private void mouseClick(double x, double y)
    {
        parentWindow.mouseClick(x, y);
    }
}
