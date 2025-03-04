package org.stephezapo.system_r.core.util;

import java.io.File;

public class DirectoryManager
{
    public static final String DATA_DIR = "data";
    public static final String CONFIG_DIR = "config";

    public static void init()
    {
        File dataDirFile = new File(DATA_DIR);
        if(!dataDirFile.exists())
        {
            dataDirFile.mkdir();
        }

        File configDirFile = new File(CONFIG_DIR);
        if(!configDirFile.exists())
        {
            configDirFile.mkdir();
        }
    }
}
