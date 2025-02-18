package org.stephezapo.system_r.core;

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