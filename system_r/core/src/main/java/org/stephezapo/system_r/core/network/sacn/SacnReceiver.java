package org.stephezapo.system_r.core.network.sacn;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;
import java.net.SocketTimeoutException;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.function.BiConsumer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.stephezapo.system_r.core.network.sacn.packet.*;

public class SacnReceiver
{
    private static final Logger logger = LoggerFactory.getLogger(SacnReceiver.class);
    private static final int BUFFER_SIZE = 1144; // large enough for max discovery packet (512 universes)

    private final MulticastSocket socket;
    private ExecutorService executor;
    private volatile boolean running;

    private final Set<String> joinedGroups = ConcurrentHashMap.newKeySet();

    private BiConsumer<SacnDataPacket, InetAddress>      dataHandler;
    private BiConsumer<SacnDiscoveryPacket, InetAddress> discoveryHandler;
    private BiConsumer<SacnSyncPacket, InetAddress>      syncHandler;

    public SacnReceiver(MulticastSocket socket)
    {
        this.socket = socket;
    }

    public void start()
    {
        if (running) return;
        running = true;
        executor = Executors.newSingleThreadExecutor(r ->
        {
            var t = new Thread(r, "sacn-receiver");
            t.setDaemon(true);
            return t;
        });
        executor.execute(this::receiveLoop);
        logger.info("sACN receiver started");
    }

    public void stop()
    {
        running = false;
        if (executor != null) executor.shutdownNow();
    }

    /** Joins the multicast group for the given universe. */
    public void subscribeUniverse(int universe) throws IOException
    {
        byte[] addr  = SacnPacket.universeToMulticast(universe);
        String group = (addr[0] & 0xFF) + "." + (addr[1] & 0xFF) + "."
                     + (addr[2] & 0xFF) + "." + (addr[3] & 0xFF);
        if (joinedGroups.add(group))
        {
            socket.joinGroup(new InetSocketAddress(InetAddress.getByName(group), 0), (NetworkInterface) null);
            logger.debug("sACN: joined multicast group {} (universe {})", group, universe);
        }
    }

    /** Leaves the multicast group for the given universe. */
    public void unsubscribeUniverse(int universe) throws IOException
    {
        byte[] addr  = SacnPacket.universeToMulticast(universe);
        String group = (addr[0] & 0xFF) + "." + (addr[1] & 0xFF) + "."
                     + (addr[2] & 0xFF) + "." + (addr[3] & 0xFF);
        if (joinedGroups.remove(group))
        {
            socket.leaveGroup(new InetSocketAddress(InetAddress.getByName(group), 0), (NetworkInterface) null);
            logger.debug("sACN: left multicast group {} (universe {})", group, universe);
        }
    }

    private void receiveLoop()
    {
        byte[] buffer = new byte[BUFFER_SIZE];
        while (running)
        {
            try
            {
                var datagram = new DatagramPacket(buffer, buffer.length);
                socket.receive(datagram);
                dispatch(datagram.getData(), datagram.getLength(), datagram.getAddress());
            }
            catch (SocketTimeoutException e)
            {
                // expected — keeps the loop interruptible
            }
            catch (IOException e)
            {
                if (running) logger.warn("sACN receive error: {}", e.getMessage());
            }
        }
    }

    private void dispatch(byte[] raw, int length, InetAddress sender)
    {
        byte[] data = new byte[length];
        System.arraycopy(raw, 0, data, 0, length);

        if (!SacnPacket.isSacn(data)) return;

        int rootVector    = SacnPacket.readRootVector(data);
        int framingVector = SacnPacket.readFramingVector(data);

        if (rootVector == SacnPacket.VECTOR_ROOT_E131_DATA
                && framingVector == SacnPacket.VECTOR_E131_DATA_PACKET)
        {
            if (dataHandler != null)
            {
                SacnDataPacket p = SacnDataPacket.fromBytes(data);
                if (p != null) dataHandler.accept(p, sender);
            }
        }
        else if (rootVector == SacnPacket.VECTOR_ROOT_E131_EXTENDED)
        {
            switch (framingVector)
            {
                case SacnPacket.VECTOR_E131_EXTENDED_DISCOVERY ->
                {
                    if (discoveryHandler != null)
                    {
                        SacnDiscoveryPacket p = SacnDiscoveryPacket.fromBytes(data);
                        if (p != null) discoveryHandler.accept(p, sender);
                    }
                }
                case SacnPacket.VECTOR_E131_EXTENDED_SYNCHRONIZATION ->
                {
                    if (syncHandler != null)
                    {
                        SacnSyncPacket p = SacnSyncPacket.fromBytes(data);
                        if (p != null) syncHandler.accept(p, sender);
                    }
                }
                default -> logger.debug("Unhandled sACN extended framing vector: 0x{}", Integer.toHexString(framingVector));
            }
        }
        else
        {
            logger.debug("Unknown sACN root vector: 0x{}", Integer.toHexString(rootVector));
        }
    }

    public void setDataHandler(BiConsumer<SacnDataPacket, InetAddress> handler)      { this.dataHandler = handler; }
    public void setDiscoveryHandler(BiConsumer<SacnDiscoveryPacket, InetAddress> handler) { this.discoveryHandler = handler; }
    public void setSyncHandler(BiConsumer<SacnSyncPacket, InetAddress> handler)      { this.syncHandler = handler; }
}
