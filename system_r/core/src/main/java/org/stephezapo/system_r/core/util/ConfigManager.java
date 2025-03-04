package org.stephezapo.system_r.core.util;

import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

public class ConfigManager
{
    private static Properties properties;

    public static void init()
    {
        properties = new Properties();

        try
        {
            properties.load(new FileInputStream("config.txt"));
            System.out.println(properties.getProperty("name"));
        }
        catch(IOException iex)
        {

        }
    }

    public static String getStringProperty(String property, String defaultValue)
    {
        return properties.getProperty(property, defaultValue);
    }

    public static int getIntProperty(String property, int defaultValue)
    {
        String value = properties.getProperty(property, "");

        try
        {
            return Integer.parseInt(value);
        }
        catch(NumberFormatException nex)
        {
            return defaultValue;
        }
    }

    public static double getDoubleProperty(String property, double defaultValue)
    {
        String value = properties.getProperty(property, "");

        try
        {
            return Double.parseDouble(value);
        }
        catch(NumberFormatException nex)
        {
            return defaultValue;
        }
    }

    public static double getShortProperty(String property, short defaultValue)
    {
        String value = properties.getProperty(property, "");

        try
        {
            return Short.parseShort(value);
        }
        catch(NumberFormatException nex)
        {
            return defaultValue;
        }
    }
}
