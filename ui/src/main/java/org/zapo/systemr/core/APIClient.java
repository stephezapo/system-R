package org.zapo.systemr.core;


import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.zapo.lumosmaxima.remote.thrift.API;


public class APIClient extends Thread
{
    private boolean running = false;
    private boolean connected = false;
    private int apiPort;
    private TTransport transport;
    private API.Client api;


    protected APIClient(int port)
    {
        apiPort = port;

        reconnect();
    }

    private void reconnect()
    {
        // disconnect
        try
        {
            transport.close();
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
        }

        // connect
        try
        {
            transport = new TSocket("localhost", apiPort);
            transport.open();
            TProtocol protocol = new  TBinaryProtocol(transport);
            api = new API.Client(protocol);

            connected = true;
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
        }
    }

    public void run()
    {
        while(running)
        {
            try
            {
                Thread.sleep(1000);

                if(connected)
                {

                }
            }
            catch(Exception ex)
            {
                ex.printStackTrace();
            }
        }
    }

    public API.Client getClient()
    {
        return api;
    }
}