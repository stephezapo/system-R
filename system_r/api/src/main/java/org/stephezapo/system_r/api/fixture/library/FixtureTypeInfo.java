package org.stephezapo.system_r.api.fixture.library;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class FixtureTypeInfo implements Serializable
{
    private String manufacturer;
    private String name;
    private String gdtfVersion;
    private String path;

    private List<FixtureTypeMode> modes = new ArrayList<>();

    public FixtureTypeInfo()
    {

    }

    public FixtureTypeInfo(String manufacturer, String name, String gdtfVersion, String path)
    {
        this.manufacturer = manufacturer;
        this.name = name;
        this.gdtfVersion = gdtfVersion;
        this.path = path;
    }

    public String getManufacturer()
    {
        return manufacturer;
    }

    public void setManufacturer(String manufacturer)
    {
        this.manufacturer = manufacturer;
    }

    public String getName()
    {
        return name;
    }

    public void setName(String name)
    {
        this.name = name;
    }

    public String getGdtfVersion()
    {
        return gdtfVersion;
    }

    public void setGdtfVersion(String gdtfVersion)
    {
        this.gdtfVersion = gdtfVersion;
    }

    public String getPath()
    {
        return path;
    }

    public void setPath(String path)
    {
        this.path = path;
    }

    public List<FixtureTypeMode> getModes()
    {
        return modes;
    }

    public void setModes(List<FixtureTypeMode> modes)
    {
        this.modes = modes;
    }

    public void addMode(FixtureTypeMode mode)
    {
        modes.add(mode);
    }
}
