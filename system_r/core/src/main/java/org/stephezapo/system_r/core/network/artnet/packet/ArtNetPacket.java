package org.stephezapo.system_r.core.network.artnet.packet;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public abstract class ArtNetPacket
{
    public static final byte[] ARTNET_ID = {'A', 'r', 't', '-', 'N', 'e', 't', 0};
    public static final int ARTNET_PORT = 6454;
    public static final short PROTOCOL_VERSION = 14;

    protected final int opCode;

    protected ArtNetPacket(int opCode)
    {
        this.opCode = opCode;
    }

    public abstract byte[] toBytes();

    protected ByteBuffer baseBuffer(int size)
    {
        ByteBuffer buf = ByteBuffer.allocate(size);
        buf.order(ByteOrder.BIG_ENDIAN);
        buf.put(ARTNET_ID);
        // OpCode is transmitted little-endian per the Art-Net specification
        buf.put((byte) (opCode & 0xFF));
        buf.put((byte) ((opCode >> 8) & 0xFF));
        return buf;
    }

    public static int readOpCode(byte[] data)
    {
        if (data.length < 10) return -1;
        return (data[8] & 0xFF) | ((data[9] & 0xFF) << 8);
    }

    public static boolean isArtNet(byte[] data)
    {
        if (data.length < 8) return false;
        for (int i = 0; i < 8; i++)
        {
            if (data[i] != ARTNET_ID[i]) return false;
        }
        return true;
    }
}
