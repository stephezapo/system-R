package org.stephezapo.system_r.core.network.artnet.packet;

import java.util.Arrays;
import org.stephezapo.system_r.core.network.artnet.ArtNetOpCode;

/**
 * Art-Net 4 ArtDmx packet.
 *
 * Universe is a 15-bit port address: bits 14-8 = Net (7 bits), bits 7-0 = SubUni (Sub 4-bit + Universe 4-bit).
 * Sequence 0 disables ordering; values 1-255 enable it.
 */
public class ArtDmxPacket extends ArtNetPacket
{
    private int sequence;
    private int physical;
    private int universe;
    private byte[] dmxData;

    public ArtDmxPacket()
    {
        super(ArtNetOpCode.OpDmx.code);
    }

    public ArtDmxPacket(int universe, byte[] dmxData, int sequence, int physical)
    {
        super(ArtNetOpCode.OpDmx.code);
        this.universe = universe & 0x7FFF;
        this.dmxData = Arrays.copyOf(dmxData, dmxData.length);
        this.sequence = sequence & 0xFF;
        this.physical = physical & 0xFF;
    }

    @Override
    public byte[] toBytes()
    {
        int dataLen = (dmxData != null) ? dmxData.length : 0;
        if (dataLen % 2 != 0) dataLen++; // spec requires even length

        var buf = baseBuffer(18 + dataLen);
        buf.put((byte) 0x00);                        // ProtVer Hi
        buf.put((byte) PROTOCOL_VERSION);             // ProtVer Lo
        buf.put((byte) sequence);
        buf.put((byte) physical);
        buf.put((byte) (universe & 0xFF));            // SubUni (low byte)
        buf.put((byte) ((universe >> 8) & 0x7F));    // Net   (high 7 bits)
        buf.put((byte) ((dataLen >> 8) & 0xFF));      // Length Hi
        buf.put((byte) (dataLen & 0xFF));             // Length Lo
        if (dmxData != null)
        {
            buf.put(dmxData);
            if (dmxData.length < dataLen) buf.put((byte) 0); // even-length pad
        }
        return buf.array();
    }

    public static ArtDmxPacket fromBytes(byte[] data)
    {
        if (data.length < 18) return null;
        var p = new ArtDmxPacket();
        p.sequence = data[12] & 0xFF;
        p.physical = data[13] & 0xFF;
        int subUni  = data[14] & 0xFF;
        int net     = data[15] & 0x7F;
        p.universe  = (net << 8) | subUni;
        int length  = ((data[16] & 0xFF) << 8) | (data[17] & 0xFF);
        int copyLen = Math.min(length, data.length - 18);
        p.dmxData   = new byte[copyLen];
        System.arraycopy(data, 18, p.dmxData, 0, copyLen);
        return p;
    }

    public int getUniverse() { return universe; }
    public void setUniverse(int universe) { this.universe = universe & 0x7FFF; }

    public byte[] getDmxData() { return dmxData; }
    public void setDmxData(byte[] dmxData) { this.dmxData = Arrays.copyOf(dmxData, dmxData.length); }

    public int getSequence() { return sequence; }
    public void setSequence(int sequence) { this.sequence = sequence & 0xFF; }

    public int getPhysical() { return physical; }
    public void setPhysical(int physical) { this.physical = physical & 0xFF; }
}
