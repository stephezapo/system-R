package org.stephezapo.system_r.core;

import org.stephezapo.system_r.api.fixture.library.LibraryInfo;
import org.stephezapo.system_r.core.fixtures.FixtureLibrary;

public class API implements IAPI
{
    public void startCore()
    {
        Core.Get().start();
    }

    public void stopCore()
    {
        Core.Get().shutdown();
    }

    public byte[] getSampleDmxData()
    {
        return Core.Get().getDmxUniverse();
    }

    @Override
    public LibraryInfo getLibraryInfo()
    {
        return FixtureLibrary.getLibraryInfo();
    }
}
