package org.stephezapo.system_r.mvrgdtf.library;

public class MvrGdtfLibrary
{
    private static LibraryData libraryData = new LibraryData();

    public static void ExtractLibrary()
    {
        libraryData.clear();
        new Thread(new LibraryCreator(libraryData)).start();
    }
}
