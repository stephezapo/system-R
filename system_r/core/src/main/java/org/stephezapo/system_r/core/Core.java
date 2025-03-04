package org.stephezapo.system_r.core;

import java.util.Locale;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.stephezapo.system_r.core.fixtures.FixtureLibrary;
import org.stephezapo.system_r.core.util.ConfigManager;
import org.stephezapo.system_r.core.util.DirectoryManager;

public class Core
{
    private static Logger logger = LoggerFactory.getLogger(Core.class);

    private final DmxUniverse dmxUniverse = new DmxUniverse();
    private static Core _instance;

    private boolean running;
    private IAPI api;

    public static void main(String[] args)
    {
        Get().start();
    }

    public static Core Get()
    {
        if(_instance == null)
        {
            _instance = new Core();
        }

        return _instance;
    }

    private Core()
    {
        api = new API();
    }

    public IAPI getAPI()
    {
        return api;
    }

    protected void start()
    {
        if(running)
        {
            return;
        }

        ConfigManager.init();
        setupLogging();
        DirectoryManager.init();
        FixtureLibrary.init();

        //logger.info("Library loaded with " + FixtureLibrary.)
        // TODO: implement startup stuff
        /*LibraryData data = new LibraryData();
        LibraryCreator creator = new LibraryCreator(data);
        new Thread(creator).start();

        while(creator.getProgress()<100)
        {
            // wait for progress to complete
        }

        System.out.println("Data collected: " + data.getData().size());
        Serializer.serialize(data, "data/LibraryData.json");
        LibraryData newData = (LibraryData)Serializer.deserialize("data/LibraryData.json", LibraryData.class);
        System.out.println("EQUAL: " + (data.equals(newData)));*/

        running = true;
    }

    protected void shutdown()
    {
        int error = 0;
        // TODO: implement shutdown stuff

        running = false;
        System.exit(error);
    }

    protected byte[] getDmxUniverse()
    {
        return dmxUniverse.getDmx();
    }

    private void setupLogging()
    {
        Locale.setDefault(Locale.ROOT);

        logger.info("Logging INFO");
        logger.debug("Logging DEBUG");
        logger.trace("Logging TRACE");
        logger.warn("Logging WARN");
        logger.error("Logging ERROR");
    }
}