package org.stephezapo.system_r.core.ui;

import javafx.geometry.Point2D;
import javafx.geometry.Rectangle2D;
import javafx.scene.layout.Background;
import javafx.scene.layout.Border;
import javafx.scene.layout.BorderStroke;
import javafx.scene.layout.BorderStrokeStyle;
import javafx.scene.layout.BorderWidths;
import javafx.scene.layout.CornerRadii;
import javafx.scene.layout.Pane;

public class Tile extends Pane
{
    private GridRect gridRect;
    private Window parent;
    private TileHeader header;

    protected Tile(Window parent)
    {
        super();
        this.parent = parent;

        setViewOrder(Double.MAX_VALUE-1);
        setBackground(Background.fill(Style.COLOR_TILE_BACKGROUND));
        setBorder(new Border(new BorderStroke(Style.COLOR_MAIN_BACKGROUND,
            BorderStrokeStyle.SOLID, new CornerRadii(0), new BorderWidths(2))));

        /*DropShadow dropShadow = new DropShadow();
        dropShadow.setRadius(10.0);
        dropShadow.setOffsetX(3.0);
        dropShadow.setOffsetY(3.0);
        dropShadow.setColor(Color.color(0.1, 0.1, 0.1));
        setEffect(dropShadow);*/

        header = new TileHeader();
        updateLayout();
        getChildren().add(header);
    }

    protected GridRect getGridRect()
    {
        return gridRect;
    }

    private void updateLayout()
    {
        header.updateLayout(getPrefWidth());
    }

    protected void moveAndScale(GridRect rect)
    {
        gridRect = rect;

        Point2D cellSize = parent.getWindowGridCellSize();
        setLayoutX(rect.getX()*cellSize.getX());
        setLayoutY(rect.getY()*cellSize.getY());

        setPrefWidth(rect.getW()*cellSize.getX());
        setPrefHeight(rect.getH()*cellSize.getY());

        updateLayout();
    }

    protected Rectangle2D getPanelRect()
    {
        return new Rectangle2D(0, header.getPrefHeight(), getPrefWidth(), getPrefHeight()-header.getPrefHeight());
    }

    protected void setTitle(String title)
    {
        header.setText(title);
    }
}