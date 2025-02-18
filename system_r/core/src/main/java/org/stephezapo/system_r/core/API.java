package org.stephezapo.system_r.core;

public class API implements IAPI
{
    public void startCore()
    {
        Core.Get().start();
    }

    public void stopCore()
    {
        Core.Get().shutdown();
    }

    public byte[] getSampleDmxData()
    {
        return Core.Get().getDmxUniverse();
    }
}
