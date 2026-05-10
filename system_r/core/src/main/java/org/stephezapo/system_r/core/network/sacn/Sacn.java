package org.stephezapo.system_r.core.network.sacn;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.function.BiConsumer;
import java.util.function.Consumer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.stephezapo.system_r.core.network.sacn.packet.SacnDataPacket;
import org.stephezapo.system_r.core.network.sacn.packet.SacnDiscoveryPacket;
import org.stephezapo.system_r.core.network.sacn.packet.SacnPacket;
import org.stephezapo.system_r.core.network.sacn.packet.SacnSyncPacket;

public class Sacn
{
    private static final Logger logger = LoggerFactory.getLogger(Sacn.class);

    private static Sacn _instance;

    private MulticastSocket        rxSocket;
    private DatagramSocket         txSocket;
    private SacnSender             sender;
    private SacnReceiver           receiver;
    private ScheduledExecutorService scheduler;

    private final byte[] cid;
    private final String sourceName;

    private final Map<String, SacnSource> discoveredSources = new ConcurrentHashMap<>();
    private final Set<Integer>            subscribedUniverses = ConcurrentHashMap.newKeySet();

    private final List<Consumer<SacnSource>>                      sourceListeners = new CopyOnWriteArrayList<>();
    private final List<BiConsumer<SacnDataPacket, SacnSource>>    dataListeners   = new CopyOnWriteArrayList<>();
    private final List<Consumer<SacnSyncPacket>>                  syncListeners   = new CopyOnWriteArrayList<>();

    private volatile boolean running;
    private volatile int     dmxSequence;
    private volatile int     syncSequence;

    public static Sacn Get()
    {
        if (_instance == null)
        {
            _instance = new Sacn("system-R", generateCid());
        }
        return _instance;
    }

    private Sacn(String sourceName, byte[] cid)
    {
        this.sourceName = sourceName;
        this.cid        = cid;
    }

    // --- Lifecycle ---

    public void start() throws IOException
    {
        if (running) return;

        // Separate sockets: MulticastSocket for receiving, DatagramSocket for sending
        rxSocket = new MulticastSocket(SacnPacket.SACN_PORT);
        rxSocket.setReuseAddress(true);
        rxSocket.setSoTimeout(500);

        txSocket = new DatagramSocket();

        sender   = new SacnSender(txSocket, cid, sourceName);
        receiver = new SacnReceiver(rxSocket);

        receiver.setDataHandler(this::onDataReceived);
        receiver.setDiscoveryHandler(this::onDiscoveryReceived);
        receiver.setSyncHandler(this::onSyncReceived);

        running = true;

        // Always subscribe to the discovery universe (bypass checkRunning — we are inside start())
        try
        {
            receiver.subscribeUniverse(SacnPacket.DISCOVERY_UNIVERSE);
            subscribedUniverses.add(SacnPacket.DISCOVERY_UNIVERSE);
        }
        catch (IOException e)
        {
            logger.warn("Could not join sACN discovery multicast group: {}", e.getMessage());
        }

        receiver.start();

        scheduler = Executors.newSingleThreadScheduledExecutor(r ->
        {
            var t = new Thread(r, "sacn-scheduler");
            t.setDaemon(true);
            return t;
        });
        // Send universe discovery every 10 seconds per E1.31-2018 §8.2
        scheduler.scheduleAtFixedRate(
            this::sendDiscovery,
            0,
            SacnDiscoveryPacket.DISCOVERY_INTERVAL_SECONDS,
            TimeUnit.SECONDS
        );

        logger.info("sACN started (CID: {})", SacnSource.cidToString(cid));
    }

    public void stop()
    {
        if (!running) return;
        running = false;

        if (scheduler != null) scheduler.shutdownNow();
        if (receiver  != null) receiver.stop();
        if (rxSocket  != null) rxSocket.close();
        if (txSocket  != null) txSocket.close();

        logger.info("sACN stopped");
    }

    // --- Sending ---

    /** Sends DMX data to the universe's multicast group. */
    public void multicastDmx(byte[] dmxData, int universe)
    {
        checkRunning();
        sender.multicastDmx(dmxData, universe, SacnPacket.DEFAULT_PRIORITY, nextDmxSequence());
    }

    /** Sends DMX data to the universe's multicast group with a specific priority (0-200). */
    public void multicastDmx(byte[] dmxData, int universe, int priority)
    {
        checkRunning();
        sender.multicastDmx(dmxData, universe, priority, nextDmxSequence());
    }

    /** Sends DMX data to a specific unicast target. */
    public void unicastDmx(InetAddress target, byte[] dmxData, int universe)
    {
        checkRunning();
        sender.unicastDmx(target, dmxData, universe, SacnPacket.DEFAULT_PRIORITY, nextDmxSequence());
    }

    /** Sends DMX data to a specific unicast target with a specific priority. */
    public void unicastDmx(InetAddress target, byte[] dmxData, int universe, int priority)
    {
        checkRunning();
        sender.unicastDmx(target, dmxData, universe, priority, nextDmxSequence());
    }

    /** Sends a synchronization packet to the given sync universe's multicast group. */
    public void sendSync(int syncUniverse)
    {
        checkRunning();
        sender.sendSync(syncUniverse, nextSyncSequence());
    }

    /** Sends stream-terminated packets for a universe (E1.31-2018 §6.2.6). */
    public void terminateStream(byte[] dmxData, int universe)
    {
        checkRunning();
        for (int i = 0; i < 3; i++)
        {
            sender.sendDmx(universeMulticast(universe), dmxData, universe,
                           SacnPacket.DEFAULT_PRIORITY, 0, nextDmxSequence(),
                           SacnPacket.OPTION_STREAM_TERMINATED);
        }
    }

    // --- Discovery / subscription ---

    /**
     * Subscribes to receive sACN data for a universe by joining its multicast group.
     * Already-subscribed universes are ignored.
     */
    public void subscribeUniverse(int universe)
    {
        checkRunning();
        if (subscribedUniverses.add(universe))
        {
            try
            {
                receiver.subscribeUniverse(universe);
            }
            catch (IOException e)
            {
                subscribedUniverses.remove(universe);
                logger.error("Failed to subscribe to sACN universe {}: {}", universe, e.getMessage());
            }
        }
    }

    /** Leaves the multicast group for a universe. */
    public void unsubscribeUniverse(int universe)
    {
        checkRunning();
        if (subscribedUniverses.remove(universe))
        {
            try
            {
                receiver.unsubscribeUniverse(universe);
            }
            catch (IOException e)
            {
                logger.error("Failed to unsubscribe from sACN universe {}: {}", universe, e.getMessage());
            }
        }
    }

    public Collection<SacnSource> getDiscoveredSources()
    {
        return Collections.unmodifiableCollection(discoveredSources.values());
    }

    public Set<Integer> getSubscribedUniverses()
    {
        return Collections.unmodifiableSet(subscribedUniverses);
    }

    public String getCidString() { return SacnSource.cidToString(cid); }
    public String getSourceName() { return sourceName; }
    public boolean isRunning()    { return running; }

    // --- Listeners ---

    public void addSourceDiscoveredListener(Consumer<SacnSource> listener)               { sourceListeners.add(listener); }
    public void removeSourceDiscoveredListener(Consumer<SacnSource> listener)            { sourceListeners.remove(listener); }

    public void addDataReceivedListener(BiConsumer<SacnDataPacket, SacnSource> listener) { dataListeners.add(listener); }
    public void removeDataReceivedListener(BiConsumer<SacnDataPacket, SacnSource> listener) { dataListeners.remove(listener); }

    public void addSyncReceivedListener(Consumer<SacnSyncPacket> listener)               { syncListeners.add(listener); }
    public void removeSyncReceivedListener(Consumer<SacnSyncPacket> listener)            { syncListeners.remove(listener); }

    // --- Internal handlers ---

    private void onDataReceived(SacnDataPacket packet, InetAddress senderAddress)
    {
        if (packet.isPreviewData()) return;

        SacnSource source = resolveSource(packet.getCid(), senderAddress, packet.getSourceName());
        for (var l : dataListeners) l.accept(packet, source);
    }

    private void onDiscoveryReceived(SacnDiscoveryPacket packet, InetAddress senderAddress)
    {
        SacnSource source = resolveSource(packet.getCid(), senderAddress, packet.getSourceName());
        source.setUniverses(packet.getUniverses());
        logger.debug("sACN discovery from {}: universes {}", source.getSourceName(), packet.getUniverses());
    }

    private void onSyncReceived(SacnSyncPacket packet, InetAddress senderAddress)
    {
        for (var l : syncListeners) l.accept(packet);
    }

    private void sendDiscovery()
    {
        if (!running) return;
        List<Integer> universes = new ArrayList<>(subscribedUniverses);
        universes.remove((Integer) SacnPacket.DISCOVERY_UNIVERSE);
        sender.sendDiscovery(universes);
        logger.debug("sACN discovery sent ({} universes)", universes.size());
    }

    private SacnSource resolveSource(byte[] cid, InetAddress address, String name)
    {
        String key = SacnSource.cidToString(cid);
        SacnSource source = discoveredSources.computeIfAbsent(key, k ->
        {
            var s = new SacnSource(cid, address, name);
            logger.info("Discovered sACN source: {}", s);
            for (var l : sourceListeners) l.accept(s);
            return s;
        });
        source.updateLastSeen();
        source.updateSourceName(name);
        return source;
    }

    private int nextDmxSequence()
    {
        int seq = (++dmxSequence) & 0xFF;
        if (seq == 0) { dmxSequence = 1; seq = 1; }
        return seq;
    }

    private int nextSyncSequence()
    {
        int seq = (++syncSequence) & 0xFF;
        if (seq == 0) { syncSequence = 1; seq = 1; }
        return seq;
    }

    private InetAddress universeMulticast(int universe)
    {
        try
        {
            return InetAddress.getByAddress(SacnPacket.universeToMulticast(universe));
        }
        catch (Exception e)
        {
            throw new IllegalStateException("Invalid universe: " + universe, e);
        }
    }

    private void checkRunning()
    {
        if (!running) throw new IllegalStateException("sACN is not running — call start() first");
    }

    private static byte[] generateCid()
    {
        UUID uuid = UUID.randomUUID();
        byte[] cid = new byte[16];
        long msb = uuid.getMostSignificantBits();
        long lsb = uuid.getLeastSignificantBits();
        for (int i = 0; i < 8; i++) cid[i]     = (byte) (msb >>> (56 - i * 8));
        for (int i = 0; i < 8; i++) cid[8 + i] = (byte) (lsb >>> (56 - i * 8));
        return cid;
    }
}
