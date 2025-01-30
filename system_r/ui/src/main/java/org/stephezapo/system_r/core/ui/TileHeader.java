package org.stephezapo.system_r.core.ui;

import static org.stephezapo.system_r.core.ui.Style.COLOR_TILE_TITLE;
import static org.stephezapo.system_r.core.ui.Style.SIZE_TILE_HEADER;
import static org.stephezapo.system_r.core.ui.Style.SIZE_TILE_TITLE_FONT;

import javafx.scene.control.Label;
import javafx.scene.layout.Background;
import javafx.scene.layout.Border;
import javafx.scene.layout.BorderStroke;
import javafx.scene.layout.BorderStrokeStyle;
import javafx.scene.layout.BorderWidths;
import javafx.scene.layout.CornerRadii;
import javafx.scene.layout.Pane;
import javafx.scene.text.Font;
import javafx.scene.text.TextAlignment;

public class TileHeader extends Pane
{
    private Label title;

    protected TileHeader()
    {
        setViewOrder(Double.MAX_VALUE-2);
        setBackground(Background.fill(Style.COLOR_TILE_HEADER));
        setBorder(new Border(new BorderStroke(Style.COLOR_ACCENT,
            BorderStrokeStyle.SOLID, new CornerRadii(0), BorderWidths.DEFAULT)));

        setPrefHeight(SIZE_TILE_HEADER);

        title = new Label("Title");
        title.setViewOrder(Double.MAX_VALUE-3);
        title.setTextFill(COLOR_TILE_TITLE);
        title.setFont(new Font(SIZE_TILE_TITLE_FONT));
        title.setTextAlignment(TextAlignment.LEFT);
        title.setLayoutX(5);
        title.setLayoutY(-1);
        title.setPrefWidth(100);
        title.setPrefHeight(SIZE_TILE_HEADER);
        getChildren().add(title);
    }

    protected void setText(String text)
    {
        title.setText(text);
    }
}
