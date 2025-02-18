package org.stephezapo.system_r.mvrgdtf.library;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class LibraryData
{
    private Map<String, List<FixtureTypeInfo>> data = new HashMap<>();

    protected LibraryData()
    {

    }

    protected void addFixtureTypeInfo(FixtureTypeInfo info)
    {
        if(!data.containsKey(info.manufacturer))
        {
            data.put(info.manufacturer, new ArrayList<>());
        }

        data.get(info.manufacturer).add(info);
    }

    protected void clear()
    {
        data.clear();
    }

    public static class FixtureTypeInfo
    {
        private String manufacturer;
        private String name;
        private String version;
        private List<Mode> modes = new ArrayList<>();

        public FixtureTypeInfo(String manufacturer, String name, String version)
        {
            this.manufacturer = manufacturer;
            this.name = name;
            this.version = version;
        }

        public void addMode(Mode mode)
        {
            modes.add(mode);
        }
    }

    public static class Mode
    {
        private String name;
        private short channelCount;

        public Mode(String name, short channelCount)
        {
            this.name = name;
            this.channelCount = channelCount;
        }
    }
}
