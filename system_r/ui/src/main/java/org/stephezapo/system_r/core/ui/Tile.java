package org.stephezapo.system_r.core.ui;

import javafx.geometry.Point2D;
import javafx.geometry.Rectangle2D;
import javafx.scene.Node;
import javafx.scene.effect.DropShadow;
import javafx.scene.layout.Background;
import javafx.scene.layout.Border;
import javafx.scene.layout.BorderStroke;
import javafx.scene.layout.BorderStrokeStyle;
import javafx.scene.layout.BorderWidths;
import javafx.scene.layout.CornerRadii;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;

public class Tile extends Pane
{
    private GridRect gridRect;
    private Window parent;

    protected Tile(Window parent)
    {
        super();
        this.parent = parent;

        setViewOrder(Double.MAX_VALUE-1);
        setBackground(Background.fill(ColorTheme.COLOR_TILE_BACKGROUND));
        setBorder(new Border(new BorderStroke(ColorTheme.COLOR_TILE_HEADER,
            BorderStrokeStyle.SOLID, new CornerRadii(5), BorderWidths.DEFAULT)));

        DropShadow dropShadow = new DropShadow();
        dropShadow.setRadius(10.0);
        dropShadow.setOffsetX(3.0);
        dropShadow.setOffsetY(3.0);
        dropShadow.setColor(Color.color(0.1, 0.1, 0.1));
        setEffect(dropShadow);
    }

    protected Rectangle2D getRect()
    {
        return new Rectangle2D(getLayoutX(), getLayoutY(), getWidth(), getHeight());
    }

    protected GridRect getGridRect()
    {
        return gridRect;
    }

    protected void moveAndScale(GridRect rect)
    {
        Point2D cellSize = parent.getWindowGridCellSize();
        setLayoutX(rect.getX()*cellSize.getX());
        setLayoutY(rect.getY()*cellSize.getY());

        setPrefWidth(rect.getW()*cellSize.getX());
        setPrefHeight(rect.getH()*cellSize.getY());

        gridRect = rect;

        System.out.println("Tile Position: " + gridRect.toString());
    }
}