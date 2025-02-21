package org.stephezapo.system_r.mvrgdtf.library;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class LibraryData implements Serializable
{
    private Map<String, List<FixtureTypeInfo>> data = new HashMap<>();

    public LibraryData()
    {

    }

    public Map<String, List<FixtureTypeInfo>> getData()
    {
        return data;
    }

    protected void addFixtureTypeInfo(FixtureTypeInfo info)
    {
        if(!data.containsKey(info.getManufacturer()))
        {
            data.put(info.getManufacturer(), new ArrayList<>());
        }

        data.get(info.getManufacturer()).add(info);
    }

    protected void clear()
    {
        data.clear();
    }
}
