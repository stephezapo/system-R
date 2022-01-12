package org.zapo.lumosmaxima.remote;

import org.apache.thrift.server.TServer;
import org.apache.thrift.server.TThreadPoolServer;
import org.apache.thrift.transport.TServerSocket;
import org.apache.thrift.transport.TTransportException;
import org.zapo.lumosmaxima.remote.thrift.API;

public class APIService extends Thread
{
    int port;
    APIServiceHandler serviceHandler;
    TServerSocket serverTransport;
    API.Processor processor;
    TThreadPoolServer.Args args;
    TServer server;

    public APIService(int listenPort) throws TTransportException
    {
        port = listenPort;

        serviceHandler = new APIServiceHandler();
        processor = new API.Processor(serviceHandler);
        serverTransport = new TServerSocket(port);
        args = new TThreadPoolServer.Args(serverTransport);
        args.processor(processor);
        //args.protocolFactory(new TBinaryProtocol.Factory(false, false,10*1024*1024));
        //args.transportFactory(new TFramedTransport.Factory());
        args.minWorkerThreads = 0;
        server = new TThreadPoolServer(args);

        setName("ClientInterface");
        System.out.println("API Service running on port " + listenPort);
    }

    @Override
    public synchronized void start()
    {
        super.start();
    }

    @Override
    public void run()
    {
        server.serve();
    }

    public void stopInterface()
    {
        server.stop();
    }
}