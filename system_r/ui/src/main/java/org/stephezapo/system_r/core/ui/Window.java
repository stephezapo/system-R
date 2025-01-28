package org.stephezapo.system_r.core.ui;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Properties;
import javafx.scene.Scene;
import javafx.stage.Stage;

public class Window extends Stage
{
    private Properties windowProps = new Properties();
    private File propsFile;

    public enum WindowType
    {
        MAIN,
        PLAYBACK1,
        PLAYBACK2,
        SPECIAL1,
        SPECIAL2,
        EXTERNAL1,
        EXTERNAL2
    }
    private MainPanel mainPanel;

    public Window(WindowType type)
    {
        String fileString = "config/Window_";
        switch(type)
        {
            case MAIN ->
            {
                setTitle("Main");
                fileString += "Main";
            }
            case PLAYBACK1 ->
            {
                setTitle("Playback 1");
                fileString += "Playback1";
            }
            case PLAYBACK2 ->
            {
                setTitle("Playback 2");
                fileString += "Playback2";
            }
            case SPECIAL1 ->
            {
                setTitle("Special 1");
                fileString += "Special1";
            }
            case SPECIAL2 ->
            {
                setTitle("Special 2");
                fileString += "Special2";
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

        propsFile = new File(fileString + "_Props.prp");

        mainPanel = new MainPanel();
        setScene(new Scene(mainPanel, 300, 250));
        loadProps();
        mainPanel.paint();
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
        try
        {
            windowProps.setProperty("window.size.width", String.valueOf(getWidth()));
            windowProps.setProperty("window.size.height", String.valueOf(getHeight()));
            windowProps.setProperty("window.pos.x", String.valueOf(getX()));
            windowProps.setProperty("window.pos.y", String.valueOf(getY()));

            windowProps.store(new FileWriter(propsFile), "");
        }
        catch(IOException ex)
        {
            ex.printStackTrace();
        }
    }
}
