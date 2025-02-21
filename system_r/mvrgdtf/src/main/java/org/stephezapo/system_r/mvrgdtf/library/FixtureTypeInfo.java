package org.stephezapo.system_r.mvrgdtf.library;

import java.util.ArrayList;
import java.util.List;

public class FixtureTypeInfo
{
    private String manufacturer;
    private String name;
    private String version;

    public String getManufacturer()
    {
        return manufacturer;
    }

    public void setManufacturer(String manufacturer)
    {
        this.manufacturer = manufacturer;
    }

    private List<FixtureTypeMode> modes = new ArrayList<>();

    public FixtureTypeInfo(String manufacturer, String name, String version)
    {
        this.manufacturer = manufacturer;
        this.name = name;
        this.version = version;
    }

    public void addMode(FixtureTypeMode mode)
    {
        modes.add(mode);
    }
}
