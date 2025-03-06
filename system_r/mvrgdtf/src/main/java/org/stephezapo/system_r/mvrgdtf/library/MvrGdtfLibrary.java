package org.stephezapo.system_r.mvrgdtf.library;

import org.stephezapo.system_r.api.fixture.library.LibraryInfo;
import org.stephezapo.system_r.api.fixture.library.LibraryState;
import org.stephezapo.system_r.api.fixture.library.LibraryState.State;

public class MvrGdtfLibrary
{
    private static LibraryInfo libraryInfo = new LibraryInfo();
    private static LibraryState libraryState = new LibraryState();
    private static LibraryCreator libraryCreator;

    public static void ExtractLibrary()
    {
        libraryInfo.clear();
        libraryState.setProgress((short)0);
        libraryState.setState(State.IMPORTING);

        libraryCreator = new LibraryCreator(libraryInfo);
        new Thread(libraryCreator).start();
    }

    public static LibraryInfo getLibraryInfo()
    {
        return libraryInfo;
    }

    public static void setLibraryData(LibraryInfo newData)
    {
        libraryInfo = newData;
        libraryState.setState(libraryInfo.getData().isEmpty() ? State.EMPTY : State.READY);
    }

    public static LibraryState getLibraryState()
    {
        if(libraryCreator != null && libraryCreator.isRunning())
        {
            libraryState.setProgress(libraryCreator.getProgress());
        }
        else
        {
            libraryState.setState(libraryInfo.getData().isEmpty() ? State.EMPTY : State.READY);
        }

        return libraryState;
    }
}
