package org.stephezapo.system_r.core.network.artnet;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketTimeoutException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.function.BiConsumer;
import java.util.function.Consumer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.stephezapo.system_r.core.network.artnet.packet.*;

public class ArtNetReceiver
{
    private static final Logger logger = LoggerFactory.getLogger(ArtNetReceiver.class);
    private static final int BUFFER_SIZE = 530;

    private final DatagramSocket socket;
    private ExecutorService executor;
    private volatile boolean running;

    private Consumer<ArtDmxPacket> dmxHandler;
    private Consumer<ArtPollPacket> pollHandler;
    private BiConsumer<ArtPollReplyPacket, InetAddress> pollReplyHandler;
    private Consumer<ArtSyncPacket> syncHandler;

    public ArtNetReceiver(DatagramSocket socket)
    {
        this.socket = socket;
    }

    public void start()
    {
        if (running) return;
        running = true;
        executor = Executors.newSingleThreadExecutor(r ->
        {
            var t = new Thread(r, "artnet-receiver");
            t.setDaemon(true);
            return t;
        });
        executor.execute(this::receiveLoop);
        logger.info("Art-Net receiver started");
    }

    public void stop()
    {
        running = false;
        if (executor != null) executor.shutdownNow();
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
                if (running) logger.warn("Art-Net receive error: {}", e.getMessage());
            }
        }
    }

    private void dispatch(byte[] raw, int length, InetAddress sender)
    {
        byte[] data = new byte[length];
        System.arraycopy(raw, 0, data, 0, length);

        if (!ArtNetPacket.isArtNet(data)) return;

        ArtNetOpCode op = ArtNetOpCode.fromCode(ArtNetPacket.readOpCode(data));
        if (op == null) return;

        switch (op)
        {
            case OpDmx ->
            {
                if (dmxHandler != null)
                {
                    ArtDmxPacket p = ArtDmxPacket.fromBytes(data);
                    if (p != null) dmxHandler.accept(p);
                }
            }
            case OpPoll ->
            {
                if (pollHandler != null)
                {
                    ArtPollPacket p = ArtPollPacket.fromBytes(data);
                    if (p != null) pollHandler.accept(p);
                }
            }
            case OpPollReply ->
            {
                if (pollReplyHandler != null)
                {
                    ArtPollReplyPacket p = ArtPollReplyPacket.fromBytes(data);
                    if (p != null) pollReplyHandler.accept(p, sender);
                }
            }
            case OpSync ->
            {
                if (syncHandler != null)
                {
                    ArtSyncPacket p = ArtSyncPacket.fromBytes(data);
                    if (p != null) syncHandler.accept(p);
                }
            }
            default -> logger.debug("Unhandled Art-Net opcode: 0x{}", Integer.toHexString(op.code));
        }
    }

    public void setDmxHandler(Consumer<ArtDmxPacket> handler) { this.dmxHandler = handler; }
    public void setPollHandler(Consumer<ArtPollPacket> handler) { this.pollHandler = handler; }
    public void setPollReplyHandler(BiConsumer<ArtPollReplyPacket, InetAddress> handler) { this.pollReplyHandler = handler; }
    public void setSyncHandler(Consumer<ArtSyncPacket> handler) { this.syncHandler = handler; }
}
