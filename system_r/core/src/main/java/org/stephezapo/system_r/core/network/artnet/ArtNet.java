package org.stephezapo.system_r.core.network.artnet;

import java.io.IOException;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.stephezapo.system_r.core.network.artnet.packet.ArtDmxPacket;
import org.stephezapo.system_r.core.network.artnet.packet.ArtNetPacket;
import org.stephezapo.system_r.core.network.artnet.packet.ArtPollPacket;
import org.stephezapo.system_r.core.network.artnet.packet.ArtPollReplyPacket;

public class ArtNet
{
    private static final Logger logger = LoggerFactory.getLogger(ArtNet.class);
    private static final int POLL_INTERVAL_SECONDS = 3;

    private static ArtNet _instance;

    private DatagramSocket socket;
    private ArtNetSender sender;
    private ArtNetReceiver receiver;
    private ScheduledExecutorService pollScheduler;

    private final Map<String, ArtNetNode> discoveredNodes      = new ConcurrentHashMap<>();
    private final List<Consumer<ArtNetNode>> nodeListeners     = new CopyOnWriteArrayList<>();
    private final List<Consumer<ArtDmxPacket>> dmxListeners    = new CopyOnWriteArrayList<>();

    private volatile boolean running;
    private volatile int dmxSequence;

    public static ArtNet Get()
    {
        if (_instance == null)
        {
            _instance = new ArtNet();
        }
        return _instance;
    }

    private ArtNet() {}

    public void start() throws IOException
    {
        if (running) return;

        socket = new DatagramSocket(ArtNetPacket.ARTNET_PORT);
        socket.setBroadcast(true);
        socket.setSoTimeout(500);

        sender   = new ArtNetSender(socket);
        receiver = new ArtNetReceiver(socket);

        receiver.setDmxHandler(this::onDmxReceived);
        receiver.setPollReplyHandler(this::onPollReplyReceived);
        receiver.setPollHandler(this::onPollReceived);
        receiver.setSyncHandler(p -> logger.debug("Art-Net sync received"));

        running = true;

        receiver.start();

        pollScheduler = Executors.newSingleThreadScheduledExecutor(r ->
        {
            var t = new Thread(r, "artnet-poll");
            t.setDaemon(true);
            return t;
        });
        pollScheduler.scheduleAtFixedRate(this::sendDiscoveryPoll, 0, POLL_INTERVAL_SECONDS, TimeUnit.SECONDS);

        logger.info("Art-Net started on port {}", ArtNetPacket.ARTNET_PORT);
    }

    public void stop()
    {
        if (!running) return;
        running = false;

        if (pollScheduler != null) pollScheduler.shutdownNow();
        if (receiver != null) receiver.stop();
        if (socket != null) socket.close();

        logger.info("Art-Net stopped");
    }

    // --- Sending ---

    public void sendDmx(InetAddress target, byte[] dmxData, int universe)
    {
        checkRunning();
        sender.sendDmx(target, dmxData, universe, nextSequence());
    }

    public void broadcastDmx(byte[] dmxData, int universe)
    {
        checkRunning();
        sender.broadcastDmx(dmxData, universe, nextSequence());
    }

    public void sendSync(InetAddress target)
    {
        checkRunning();
        sender.sendSync(target);
    }

    public void broadcastSync()
    {
        checkRunning();
        sender.broadcastSync();
    }

    // --- Discovery ---

    public void sendDiscoveryPoll()
    {
        if (!running) return;
        sender.sendPoll();
        logger.debug("Art-Net poll broadcast sent");
    }

    public Collection<ArtNetNode> getDiscoveredNodes()
    {
        return Collections.unmodifiableCollection(discoveredNodes.values());
    }

    // --- Listeners ---

    public void addNodeDiscoveredListener(Consumer<ArtNetNode> listener) { nodeListeners.add(listener); }
    public void removeNodeDiscoveredListener(Consumer<ArtNetNode> listener) { nodeListeners.remove(listener); }

    public void addDmxReceivedListener(Consumer<ArtDmxPacket> listener) { dmxListeners.add(listener); }
    public void removeDmxReceivedListener(Consumer<ArtDmxPacket> listener) { dmxListeners.remove(listener); }

    public boolean isRunning() { return running; }

    // --- Internal handlers ---

    private void onDmxReceived(ArtDmxPacket packet)
    {
        for (Consumer<ArtDmxPacket> l : dmxListeners) l.accept(packet);
    }

    private void onPollReplyReceived(ArtPollReplyPacket reply, InetAddress senderAddress)
    {
        String key = senderAddress.getHostAddress();
        ArtNetNode existing = discoveredNodes.get(key);
        if (existing != null)
        {
            existing.updateLastSeen();
            return;
        }

        ArtNetNode node = new ArtNetNode(senderAddress, reply);
        discoveredNodes.put(key, node);
        logger.info("Discovered Art-Net node: {}", node);
        for (Consumer<ArtNetNode> l : nodeListeners) l.accept(node);
    }

    private void onPollReceived(ArtPollPacket poll)
    {
        // This app acts as a controller, not a node — no reply needed.
        logger.debug("Art-Net poll received");
    }

    private int nextSequence()
    {
        int seq = dmxSequence++ & 0xFF;
        if (seq == 0) seq = 1; // 0 disables ordering per the spec
        dmxSequence = seq;
        return seq;
    }

    private void checkRunning()
    {
        if (!running) throw new IllegalStateException("Art-Net is not running — call start() first");
    }
}
