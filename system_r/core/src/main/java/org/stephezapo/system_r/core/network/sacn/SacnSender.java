package org.stephezapo.system_r.core.network.sacn;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.List;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.stephezapo.system_r.core.network.sacn.packet.SacnDataPacket;
import org.stephezapo.system_r.core.network.sacn.packet.SacnDiscoveryPacket;
import org.stephezapo.system_r.core.network.sacn.packet.SacnPacket;
import org.stephezapo.system_r.core.network.sacn.packet.SacnSyncPacket;

public class SacnSender
{
    private static final Logger logger = LoggerFactory.getLogger(SacnSender.class);

    private final DatagramSocket socket;
    private final byte[]         cid;
    private final String         sourceName;

    public SacnSender(DatagramSocket socket, byte[] cid, String sourceName)
    {
        this.socket     = socket;
        this.cid        = cid;
        this.sourceName = sourceName;
    }

    /** Sends DMX data to the universe's multicast group. */
    public void multicastDmx(byte[] dmxData, int universe, int priority, int sequence)
    {
        InetAddress target = universeMulticast(universe);
        sendDmx(target, dmxData, universe, priority, 0, sequence, 0);
    }

    /** Sends DMX data to a specific unicast target. */
    public void unicastDmx(InetAddress target, byte[] dmxData, int universe, int priority, int sequence)
    {
        sendDmx(target, dmxData, universe, priority, 0, sequence, 0);
    }

    /**
     * Sends DMX data with full control over all framing fields.
     * syncAddress = 0 means no synchronization.
     */
    public void sendDmx(InetAddress target, byte[] dmxData, int universe, int priority,
                        int syncAddress, int sequence, int options)
    {
        var packet = new SacnDataPacket(cid, sourceName, universe, dmxData,
                                        priority, syncAddress, sequence, options);
        send(packet.toBytes(), target);
    }

    /** Sends a sync packet to the given synchronization universe's multicast group. */
    public void sendSync(int syncUniverse, int sequence)
    {
        InetAddress target = universeMulticast(syncUniverse);
        var packet = new SacnSyncPacket(cid, sequence, syncUniverse);
        send(packet.toBytes(), target);
    }

    /** Broadcasts universe discovery pages to the discovery multicast group (239.255.250.222). */
    public void sendDiscovery(List<Integer> universes)
    {
        InetAddress target = universeMulticast(SacnPacket.DISCOVERY_UNIVERSE);
        List<SacnDiscoveryPacket> pages = SacnDiscoveryPacket.createPages(cid, sourceName, universes);
        for (SacnDiscoveryPacket page : pages)
        {
            send(page.toBytes(), target);
        }
    }

    private void send(byte[] data, InetAddress address)
    {
        try
        {
            socket.send(new DatagramPacket(data, data.length, address, SacnPacket.SACN_PORT));
        }
        catch (IOException e)
        {
            logger.error("Failed to send sACN packet to {}: {}", address.getHostAddress(), e.getMessage());
        }
    }

    private InetAddress universeMulticast(int universe)
    {
        byte[] addr = SacnPacket.universeToMulticast(universe);
        try
        {
            return InetAddress.getByAddress(addr);
        }
        catch (UnknownHostException e)
        {
            throw new IllegalStateException("Invalid multicast address for universe " + universe, e);
        }
    }
}
