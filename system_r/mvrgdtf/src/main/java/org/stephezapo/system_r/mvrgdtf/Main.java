package org.stephezapo.system_r.mvrgdtf;

import org.stephezapo.system_r.mvrgdtf.library.LibraryCreator;

public class Main
{
    public static void main(String[] args) {
        new Thread(new LibraryCreator()).start();
    }
}