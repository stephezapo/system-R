package org.stephezapo.system_r.core;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.xml.XmlMapper;
import org.stephezapo.system_r.mvrgdtf.library.LibraryCreator;
import org.stephezapo.system_r.mvrgdtf.library.LibraryData;

public class Core
{
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

        // TODO: implement startup stuff
        LibraryData data = new LibraryData();
        LibraryCreator creator = new LibraryCreator(data);
        new Thread(creator).start();

        while(creator.getProgress()<100)
        {
            // wait for progress to complete
        }

        System.out.println("Data collected: " + data.getData().size());
        XmlMapper xmlMapper = new XmlMapper();
        try
        {
            /*String xml = xmlMapper.writeValueAsString(data);

            LibraryData value
                = xmlMapper.readValue(xml, LibraryData.class);
            System.out.println(xml);*/

            ObjectMapper mapper = new ObjectMapper();
            String jsonResult = mapper.writerWithDefaultPrettyPrinter()
                .writeValueAsString(data.getData());
            System.out.println(jsonResult);
        }
        catch (JsonProcessingException e)
        {
            throw new RuntimeException(e);
        }

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
}