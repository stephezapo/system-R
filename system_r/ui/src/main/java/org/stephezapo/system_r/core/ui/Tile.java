package org.stephezapo.system_r.core.ui;

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
    protected Tile()
    {
        super();
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
}