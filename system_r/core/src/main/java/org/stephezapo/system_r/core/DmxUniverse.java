package org.stephezapo.system_r.core;

import java.util.Random;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

public class DmxUniverse
{
    private ScheduledExecutorService executorService;
    private ScheduledFuture<?> scheduledFuture;
    private byte[] dmx = new byte[512];
    public DmxUniverse()
    {
        executorService = Executors.newScheduledThreadPool(1);
        scheduledFuture = executorService.scheduleAtFixedRate(this::randomDmx, 100, 20, TimeUnit.MILLISECONDS);
    }

    private void randomDmx()
    {
        Random rand = new Random();
        synchronized (dmx)
        {
            for(int i = 0; i<dmx.length; i++)
            {
                dmx[i] = (byte)rand.nextInt(255);
            }
        }
    }

    public byte[] getDmx()
    {
        synchronized (dmx)
        {
            return dmx;
        }
    }
}
