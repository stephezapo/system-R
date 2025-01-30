package org.stephezapo.system_r.core.ui;

import static org.stephezapo.system_r.core.ui.Style.COLOR_GRID_POINT;
import static org.stephezapo.system_r.core.ui.Style.COLOR_MAIN_BACKGROUND;
import static org.stephezapo.system_r.core.ui.Style.SIZE_GRID_POINT_RADIUS;
import static org.stephezapo.system_r.core.ui.Style.SIZE_GRID_POINT_SPACING;

import javafx.geometry.Point2D;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Background;
import javafx.scene.layout.Pane;
import javafx.scene.shape.Circle;

public class GridPanel
    extends Pane
{
    private GridPoint gridSize = new GridPoint();
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

        gridSize = new GridPoint((int)Math.round(width/ SIZE_GRID_POINT_SPACING),
            (int)Math.round(height/ SIZE_GRID_POINT_SPACING));

        cellSize = new Point2D(width/gridSize.getX(), height/gridSize.getY());

        // Draw points in a regular square pattern
        for (double x = cellSize.getX(); x < width; x += cellSize.getX())
        {
            for (double y = cellSize.getY(); y < height; y += cellSize.getY())
            {
                Circle circle = new Circle(x, y, SIZE_GRID_POINT_RADIUS, COLOR_GRID_POINT);
                circle.setViewOrder(Double.MAX_VALUE);
                getChildren().add(circle);
            }
        }
    }

    protected GridPoint getGridSize()
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
