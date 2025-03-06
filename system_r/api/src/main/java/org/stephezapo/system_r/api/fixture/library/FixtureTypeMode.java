package org.stephezapo.system_r.api.fixture.library;

import java.io.Serializable;

public class FixtureTypeMode implements Serializable
{
    private String name;
    private short channelCount;

    public String getName()
    {
        return name;
    }

    public void setName(String name)
    {
        this.name = name;
    }

    public short getChannelCount()
    {
        return channelCount;
    }

    public void setChannelCount(short channelCount)
    {
        this.channelCount = channelCount;
    }

    public FixtureTypeMode()
    {

    }

    public FixtureTypeMode(String name, short channelCount)
    {
        this.name = name;
        this.channelCount = channelCount;
    }
}
