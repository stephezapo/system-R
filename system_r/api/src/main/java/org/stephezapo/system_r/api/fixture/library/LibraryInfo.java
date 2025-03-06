package org.stephezapo.system_r.api.fixture.library;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class LibraryInfo
    implements Serializable
{
    private Map<String, List<FixtureTypeInfo>> data = new HashMap<>();

    public LibraryInfo()
    {

    }

    public Map<String, List<FixtureTypeInfo>> getData()
    {
        return data;
    }

    public void addFixtureTypeInfo(FixtureTypeInfo info)
    {
        if(!data.containsKey(info.getManufacturer()))
        {
            data.put(info.getManufacturer(), new ArrayList<>());
        }

        data.get(info.getManufacturer()).add(info);
    }

    public void clear()
    {
        data.clear();
    }
}
