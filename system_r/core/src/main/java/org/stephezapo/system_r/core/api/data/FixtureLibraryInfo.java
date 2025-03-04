package org.stephezapo.system_r.core.api.data;

public class FixtureLibraryInfo
{
    public enum LibraryState
    {
        EMPTY,
        READY,
        IMPORTING
    }

    private LibraryState state;
    short progress;

    public FixtureLibraryInfo()
    {
        state = LibraryState.EMPTY;
        progress = 0;
    }

    public void setState(LibraryState state)
    {
        this.state = state;
    }
}