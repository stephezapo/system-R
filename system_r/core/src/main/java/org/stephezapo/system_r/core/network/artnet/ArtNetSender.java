package org.stephezapo.system_r.core.network.artnet;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.stephezapo.system_r.core.network.artnet.packet.*;

public class ArtNetSender
{
    private static final Logger logger = LoggerFactory.getLogger(ArtNetSender.class);

    private static final InetAddress BROADCAST;

    static
    {
        try
        {
            BROADCAST = InetAddress.getByName("255.255.255.255");
        }
        catch (UnknownHostException e)
        {
            throw new ExceptionInInitializerError(e);
        }
    }

    private final DatagramSocket socket;

    public ArtNetSender(DatagramSocket socket)
    {
        this.socket = socket;
    }

    public void sendDmx(InetAddress target, byte[] dmxData, int universe, int sequence)
    {
        send(new ArtDmxPacket(universe, dmxData, sequence, 0), target);
    }

    public void broadcastDmx(byte[] dmxData, int universe, int sequence)
    {
        sendDmx(BROADCAST, dmxData, universe, sequence);
    }

    public void sendPoll()
    {
        send(new ArtPollPacket(), BROADCAST);
    }

    public void sendPollReply(InetAddress target, ArtPollReplyPacket reply)
    {
        send(reply, target);
    }

    public void broadcastPollReply(ArtPollReplyPacket reply)
    {
        send(reply, BROADCAST);
    }

    public void sendSync(InetAddress target)
    {
        send(new ArtSyncPacket(), target);
    }

    public void broadcastSync()
    {
        send(new ArtSyncPacket(), BROADCAST);
    }

    private void send(ArtNetPacket packet, InetAddress address)
    {
        try
        {
            byte[] data = packet.toBytes();
            socket.send(new DatagramPacket(data, data.length, address, ArtNetPacket.ARTNET_PORT));
        }
        catch (IOException e)
        {
            logger.error("Failed to send Art-Net packet to {}: {}", address.getHostAddress(), e.getMessage());
        }
    }
}
