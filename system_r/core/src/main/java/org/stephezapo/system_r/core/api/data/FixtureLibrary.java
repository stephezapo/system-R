package org.stephezapo.system_r.core.api.data;

import java.io.File;

public class FixtureLibrary
{
    public enum LibraryState
    {
        EMPTY,
        READY,
        IMPORTING
    }

    public static class FixtureLibraryStatus
    {
        short progress;
        LibraryState state;

        public FixtureLibraryStatus()
        {
            progress = 0;
            state = LibraryState.EMPTY;
        }
    }

    private void loadLibraryDB()
    {

    }
}