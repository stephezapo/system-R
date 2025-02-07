package org.stephezapo.system_r.core.ui;

import javafx.scene.effect.DropShadow;
import javafx.scene.layout.Background;
import javafx.scene.layout.Border;
import javafx.scene.layout.BorderStroke;
import javafx.scene.layout.BorderStrokeStyle;
import javafx.scene.layout.BorderWidths;
import javafx.scene.layout.CornerRadii;
import javafx.scene.paint.Color;

public class Dialog extends Tile
{
    private static final double DIALOG_WIDTH = 600.0;
    private static final double DIALOG_HEIGHT = 200.0;

    public enum DialogType
    {
        NEW_TILE,
        ERROR,
        INFO,
        INPUT_TEXT,
        INPUT_NUMBER
    }
    private DialogType type;

    protected Dialog(Window parent, String title)
    {
        super(parent);

        setViewOrder(Double.MAX_VALUE-10);
        setBackground(Background.fill(Style.COLOR_TILE_BACKGROUND));
        setBorder(new Border(new BorderStroke(Style.COLOR_MAIN_BACKGROUND,
            BorderStrokeStyle.SOLID, new CornerRadii(0), new BorderWidths(2))));

        DropShadow dropShadow = new DropShadow();
        dropShadow.setRadius(20.0);
        dropShadow.setOffsetX(5.0);
        dropShadow.setOffsetY(5.0);
        dropShadow.setColor(Color.color(0.1, 0.1, 0.1));
        setEffect(dropShadow);


        setPrefWidth(DIALOG_WIDTH);
        setPrefHeight(DIALOG_HEIGHT);
        setLayoutX(parent.getWidth()/2.0-DIALOG_WIDTH/2.0);
        setLayoutY(parent.getHeight()/2.0-DIALOG_HEIGHT/2.0);

        updateLayout();
        setTitle(title);
    }

    protected void close()
    {
        parent.closeDialog(this);
    }
}
