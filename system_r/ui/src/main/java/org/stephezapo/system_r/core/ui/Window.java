package org.stephezapo.system_r.core.ui;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import javafx.geometry.Point2D;
import javafx.scene.Group;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.stage.Stage;
import org.stephezapo.system_r.core.Main;
import org.stephezapo.system_r.core.ui.Dialog.DialogType;

public class Window extends Stage
{
    private WindowType type;
    private Properties windowProps = new Properties();
    private File propsFile;
    private AtomicBoolean storeFlag = new AtomicBoolean(false);
    private ScheduledExecutorService executorService;
    private ScheduledFuture<?> scheduledFuture;
    private final Group tiles = new Group();
    private Point2D lastClick = new Point2D(0, 0);
    private boolean dialogIsOpen = false;
    private static final int INTERNAL_SCREEN_WIDTH=800;
    private static final int INTERNAL_SCREEN_HEIGHT=480;

    public enum WindowType
    {
        PLAYBACK1,
        PLAYBACK2,
        PROGRAMMER,
        EXTERNAL1,
        EXTERNAL2
    }

    private GridPanel gridPanel;

    public Window(WindowType type)
    {
        this.type = type;

        String fileString = "config/Window_";
        switch(type)
        {
            case PLAYBACK1 ->
            {
                if(Main.DEVMODE)
                {
                    setX(10);
                    setY(10);
                }
                setTitle("Playback 1");
                fileString += "Playback1";
            }
            case PLAYBACK2 ->
            {
                setTitle("Playback 2");
                fileString += "Playback2";
            }
            case PROGRAMMER ->
            {
                if(Main.DEVMODE)
                {
                    setX(INTERNAL_SCREEN_WIDTH+20);
                    setY(10);
                }
                setTitle("Programmer");
                fileString += "Programmer";
            }
            case EXTERNAL1 ->
            {
                setTitle("External 1");
                fileString += "External1";
            }
            case EXTERNAL2 ->
            {
                setTitle("External 2");
                fileString += "External2";
            }
        }

        if(type!=WindowType.EXTERNAL1 && type!=WindowType.EXTERNAL2)
        {
            setWidth(INTERNAL_SCREEN_WIDTH);
            setHeight(INTERNAL_SCREEN_HEIGHT);
        }

        propsFile = new File(fileString + "_Props.prp");

        gridPanel = new GridPanel(this);
        setScene(new Scene(gridPanel, 300, 250));

        gridPanel.getChildren().add(tiles);

        getScene().widthProperty().addListener((observableValue, number, t1) -> resize());
        getScene().heightProperty().addListener((observableValue, number, t1) -> resize());
        maximizedProperty().addListener((observableValue, number, t1) -> resize());
        xProperty().addListener((observableValue, number, t1) -> move());
        yProperty().addListener((observableValue, number, t1) -> move());

        setOnCloseRequest(windowEvent -> close());

        loadProps();

        executorService = Executors.newScheduledThreadPool(1);
        scheduledFuture = executorService.scheduleAtFixedRate(this::storeProps, 1, 5, TimeUnit.SECONDS);

        resize();
    }

    public void close()
    {
        if(scheduledFuture != null)
        {
            scheduledFuture.cancel(false);
            WindowManager.closeWindow(type);
        }
    }

    public void showDialog(DialogType type)
    {
        Dialog dialog;

        switch(type)
        {
            case NEW_TILE:
                dialog = new NewTileDialog(this);
                break;
            default:
                dialog = new Dialog(this, "Dialog");
                break;
        }

        dialogIsOpen = true;
        gridPanel.getChildren().add(dialog);
    }

    protected void closeTile(Tile tile)
    {
        tiles.getChildren().remove(tile);
    }

    protected void closeDialog(Dialog dialog)
    {
        gridPanel.getChildren().remove(dialog);
        dialogIsOpen = false;
    }

    protected GridPoint getWindowGridSize()
    {
        return gridPanel.getGridSize();
    }

    protected Point2D getWindowGridCellSize()
    {
        return gridPanel.getGridCellSize();
    }

    protected GridPoint getGridPointFromXY(double x, double y)
    {
        Point2D cellSize = getWindowGridCellSize();

        return new GridPoint((short)Math.floor(x/cellSize.getX()),
            (short)Math.floor(y/cellSize.getY()));
    }

    protected void mouseClick(double x, double y)
    {
        if(dialogIsOpen)
        {
            return;
        }

        lastClick = new Point2D(x, y);

        // TODO: show Dialog which Tile shall be created
        GridPoint mousePoint = getGridPointFromXY(x, y);

        // First check if there is no tile at the mouse position
        if(gridCellOccupied(mousePoint))
        {
            return;
        }

        showDialog(DialogType.NEW_TILE);
    }

    protected void addTile(Tile newTile)
    {
        GridPoint mousePoint = getGridPointFromXY(lastClick.getX(), lastClick.getY());
        GridRect fittedRectangle = getFittedRectangle(mousePoint);
        newTile.moveAndScale(fittedRectangle);

        tiles.getChildren().add(newTile);
    }

    private GridRect getFittedRectangle(GridPoint position)
    {
        // find the largest possible dimension
        // strategy: start with a rect with height 1 and maximum width and find the right-side limit
        // increase height iteratively and check again for the right-side limit
        GridPoint gridSize = getWindowGridSize();
        int maxHeight = 0;
        int maxWidth = 0;

        // find the max height of a single column
        for(int gy = position.getY(); gy < gridSize.getY(); gy++)
        {
            if(gridCellOccupied(new GridPoint(position.getX(), gy)))
            {
                break;
            }

            maxHeight += 1;
        }

        // find the max width of a single row
        for(int gx = position.getX(); gx < gridSize.getX(); gx++)
        {
            if(gridCellOccupied(new GridPoint(gx, position.getY())))
            {
                break;
            }

            maxWidth += 1;
        }

        // start with a rect with maximum width and height=1 and iteratively grow in height
        if(maxWidth > 1 && maxHeight > 1)
        {
            int newWidth = maxWidth;
            int newHeight = 0;
            boolean stop = false;
            for(int y = position.getY(); y < position.getY() + maxHeight; y++)
            {
                for(int x = position.getX(); x < position.getX() + maxWidth; x++)
                {
                    if(gridCellOccupied(new GridPoint(x, y)))
                    {
                        // the newly truncated rect would be higher than wider -> stop
                        if(x - position.getX() < maxHeight + 1)
                        {
                            stop = true;
                        }
                        else
                        {
                            maxWidth = x - position.getX();
                        }

                        break;
                    }
                }

                if(stop)
                {
                    break;
                }

                newHeight += 1;
            }

            return new GridRect(position.getX(), position.getY(),
                maxWidth, newHeight);
        }

        return new GridRect(position.getX(), position.getY(),
            maxWidth, maxHeight);
    }

    private boolean gridCellOccupied(GridPoint point)
    {
        boolean occupied = false;
        for(Node node : tiles.getChildren())
        {
            if(!(node instanceof Dialog) && ((Tile)node).getGridRect().contains(point))
            {
                return true;
            }
        }

        return occupied;
    }

    private void loadProps()
    {
        try
        {
            if(propsFile.exists())
            {
                windowProps.load(new FileInputStream(propsFile.getAbsoluteFile()));
            }
            else
            {
                windowProps = new Properties();
            }

            setWidth(Double.parseDouble(windowProps.getProperty("window.size.width", "400")));
            setHeight(Double.parseDouble(windowProps.getProperty("window.size.height", "300")));
            setMaximized(Boolean.parseBoolean(windowProps.getProperty("window.state.maximized", "false")));
            setX(Double.parseDouble(windowProps.getProperty("window.pos.x", "200")));
            setY(Double.parseDouble(windowProps.getProperty("window.pos.y", "200")));
        }
        catch(IOException ex)
        {
            ex.printStackTrace();
        }
    }

    private void storeProps()
    {
        if(!storeFlag.get())
        {
            return;
        }

        try
        {
            windowProps.setProperty("window.size.width", String.valueOf(getWidth()));
            windowProps.setProperty("window.size.height", String.valueOf(getHeight()));
            windowProps.setProperty("window.state.maximized", String.valueOf(maximizedProperty().get()));
            windowProps.setProperty("window.pos.x", String.valueOf(getX()));
            windowProps.setProperty("window.pos.y", String.valueOf(getY()));

            if(!propsFile.exists())
            {
                if(!propsFile.getParentFile().exists())
                {
                    if(!propsFile.getParentFile().mkdirs())
                    {
                        throw new IOException("Could not create parent directory");
                    }
                }

                if(!propsFile.createNewFile())
                {
                    throw new IOException("Could not create property file for " + getTitle());
                }
            }

            windowProps.store(new FileWriter(propsFile), "");

            storeFlag.set(false);
        }
        catch(IOException ex)
        {
            ex.printStackTrace();
        }
    }

    private void resize()
    {
        gridPanel.redraw();
        storeFlag.set(true);
    }

    private void move()
    {
        storeFlag.set(true);
    }
}
