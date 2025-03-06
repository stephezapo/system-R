package org.stephezapo.system_r.core;

import org.stephezapo.system_r.api.fixture.library.LibraryInfo;

public interface IAPI
{
    // Basic Core Functions
    void startCore();
    void stopCore();

    // Fixture Library


    // DMX
    byte[] getSampleDmxData();

    LibraryInfo getLibraryInfo();
}
