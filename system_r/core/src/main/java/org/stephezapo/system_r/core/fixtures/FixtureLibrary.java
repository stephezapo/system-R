package org.stephezapo.system_r.core.fixtures;

import java.io.File;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.stephezapo.system_r.core.api.data.FixtureLibraryInfo;
import org.stephezapo.system_r.core.api.data.FixtureLibraryInfo.LibraryState;
import org.stephezapo.system_r.core.util.ConfigManager;
import org.stephezapo.system_r.core.util.Serializer;
import org.stephezapo.system_r.mvrgdtf.library.LibraryData;
import org.stephezapo.system_r.mvrgdtf.library.MvrGdtfLibrary;

public class FixtureLibrary
{
    private static Logger logger = LoggerFactory.getLogger(FixtureLibrary.class);
    private static FixtureLibraryInfo libraryInfo = new FixtureLibraryInfo();

    public static void init()
    {
        loadDatabaseFromFile();
    }

    /**
     * Load library database from a JSON file
     */
    public static void loadDatabaseFromFile()
    {
        String fileName = ConfigManager.getStringProperty("data.library.file","");

        if(fileName.isEmpty())
        {
            logger.warn("Could not load library database file, because the config line is missing");
            return;
        }

        File databaseFile = new File(fileName);

        if(!databaseFile.exists())
        {
            logger.warn("Could not load library database file, because the file does not exist");
        }

        LibraryData data = (LibraryData)Serializer.deserialize(fileName, LibraryData.class);

        if(data != null)
        {
            MvrGdtfLibrary.setLibraryData(data);
            if(data.getData().isEmpty())
            {
                libraryInfo.setState(LibraryState.EMPTY);
                logger.info("Library database successfully loaded, but the database is empty");
            }
            else
            {
                libraryInfo.setState(LibraryState.READY);
                logger.info("Library database successfully loaded");
            }
        }
        else
        {
            logger.warn("Could not load library database file");
        }
    }
}
