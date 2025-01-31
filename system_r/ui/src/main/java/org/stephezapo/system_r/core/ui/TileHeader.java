package org.stephezapo.system_r.core.ui;

import static org.stephezapo.system_r.core.ui.Style.COLOR_TILE_CLOSE_BUTTON;
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
import javafx.scene.shape.Circle;
import javafx.scene.text.Font;
import javafx.scene.text.TextAlignment;

public class TileHeader extends Pane
{
    private Label title;
    private Circle closeButton;

    protected TileHeader()
    {
        setViewOrder(Double.MAX_VALUE-2);
        setBackground(Background.fill(Style.COLOR_TILE_HEADER));
        setBorder(new Border(new BorderStroke(Style.COLOR_MAIN_BACKGROUND,
            BorderStrokeStyle.SOLID, new CornerRadii(0), new BorderWidths(2))));

        setPrefHeight(SIZE_TILE_HEADER);

        title = new Label("");
        title.setViewOrder(Double.MAX_VALUE-3);
        title.setTextFill(COLOR_TILE_TITLE);
        title.setFont(new Font(SIZE_TILE_TITLE_FONT));
        title.setTextAlignment(TextAlignment.LEFT);
        title.setLayoutX(5);
        title.setLayoutY(-1);
        title.setPrefWidth(100);
        title.setPrefHeight(SIZE_TILE_HEADER);
        getChildren().add(title);

        double radius = SIZE_TILE_HEADER/4.0;
        closeButton = new Circle(SIZE_TILE_HEADER, SIZE_TILE_HEADER/2.0, radius, COLOR_TILE_CLOSE_BUTTON);
        closeButton.setViewOrder(Double.MAX_VALUE-4);
        getChildren().add(closeButton);
    }

    protected void setText(String text)
    {
        title.setText(text);
    }

    protected void updateLayout(double width)
    {
        setLayoutX(0);
        setLayoutY(0);
        setPrefWidth(width);

        closeButton.setCenterX(width - SIZE_TILE_HEADER/2.0);
    }
}
