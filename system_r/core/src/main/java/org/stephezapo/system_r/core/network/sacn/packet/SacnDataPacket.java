package org.stephezapo.system_r.core.network.sacn.packet;

import java.nio.ByteBuffer;
import java.util.Arrays;

/**
 * E1.31 Data Packet — carries DMX512 values for one universe.
 *
 * Packet layout (offsets from byte 0):
 *   [0-15]   Preamble
 *   [16-37]  Root PDU  (flags+len, VECTOR_ROOT_E131_DATA, CID)
 *   [38-114] Framing PDU (flags+len, VECTOR_E131_DATA_PACKET, source name,
 *                         priority, sync address, sequence, options, universe)
 *   [115+]   DMP PDU   (flags+len, vector, addr type, first addr, addr inc,
 *                         property count, start code, DMX data)
 */
public class SacnDataPacket extends SacnPacket
{
    private final String sourceName;
    private final int    priority;
    private final int    syncAddress;
    private final int    sequence;
    private final int    options;
    private final int    universe;
    private final byte[] dmxData;

    public SacnDataPacket(byte[] cid, String sourceName, int universe, byte[] dmxData,
                          int priority, int syncAddress, int sequence, int options)
    {
        super(cid);
        this.sourceName  = sourceName;
        this.universe    = universe & 0xFFFF;
        this.dmxData     = Arrays.copyOf(dmxData, dmxData.length);
        this.priority    = priority;
        this.syncAddress = syncAddress;
        this.sequence    = sequence & 0xFF;
        this.options     = options & 0xFF;
    }

    @Override
    public byte[] toBytes()
    {
        int channels     = dmxData.length;
        int propCount    = channels + 1; // +1 for DMX start code

        // DMP PDU: 2 (flags+len) + 1 (vec) + 1 (addr type) + 2 (first addr) +
        //          2 (addr inc) + 2 (prop count) + 1 (start code) + channels
        int dmpLen      = 11 + channels;

        // Framing PDU: 2 (flags+len) + 4 (vec) + 64 (name) + 1 (prio) +
        //              2 (sync) + 1 (seq) + 1 (options) + 2 (universe) + dmpLen
        int framingLen  = 77 + dmpLen;

        // Root PDU: 2 (flags+len) + 4 (vec) + 16 (CID) + framingLen
        int rootLen     = 22 + framingLen;

        // Total: 16 (preamble) + rootLen
        int totalLen    = 16 + rootLen;

        ByteBuffer buf = ByteBuffer.allocate(totalLen);

        writePreamble(buf);
        writeRootHeader(buf, rootLen, VECTOR_ROOT_E131_DATA, cid);

        // Framing PDU
        buf.putShort((short) flagsLen(framingLen));
        buf.putInt(VECTOR_E131_DATA_PACKET);
        writeSourceName(buf, sourceName);
        buf.put((byte) priority);
        buf.putShort((short) syncAddress);
        buf.put((byte) sequence);
        buf.put((byte) options);
        buf.putShort((short) universe);

        // DMP PDU
        buf.putShort((short) flagsLen(dmpLen));
        buf.put((byte) VECTOR_DMP_SET_PROPERTY);
        buf.put((byte) ADDRESS_TYPE_DMX512);
        buf.putShort((short) 0x0000); // first property address
        buf.putShort((short) 0x0001); // address increment
        buf.putShort((short) propCount);
        buf.put((byte) 0x00); // DMX512 start code
        buf.put(dmxData);

        return buf.array();
    }

    public static SacnDataPacket fromBytes(byte[] data)
    {
        if (data.length < 126) return null;

        byte[] cid = new byte[16];
        System.arraycopy(data, 22, cid, 0, 16);

        String sourceName  = readSourceName(data, 44, 64);
        int    priority    = data[108] & 0xFF;
        int    syncAddress = ((data[109] & 0xFF) << 8) | (data[110] & 0xFF);
        int    sequence    = data[111] & 0xFF;
        int    options     = data[112] & 0xFF;
        int    universe    = ((data[113] & 0xFF) << 8) | (data[114] & 0xFF);

        int    propCount   = ((data[123] & 0xFF) << 8) | (data[124] & 0xFF);
        int    channels    = Math.max(0, propCount - 1); // subtract start code
        int    copyLen     = Math.min(channels, data.length - 126);

        byte[] dmxData = new byte[copyLen];
        if (copyLen > 0) System.arraycopy(data, 126, dmxData, 0, copyLen);

        return new SacnDataPacket(cid, sourceName, universe, dmxData,
                                  priority, syncAddress, sequence, options);
    }

    public String getSourceName()  { return sourceName; }
    public int    getPriority()    { return priority; }
    public int    getSyncAddress() { return syncAddress; }
    public int    getSequence()    { return sequence; }
    public int    getOptions()     { return options; }
    public int    getUniverse()    { return universe; }
    public byte[] getDmxData()     { return dmxData; }

    public boolean isStreamTerminated()    { return (options & OPTION_STREAM_TERMINATED) != 0; }
    public boolean isPreviewData()         { return (options & OPTION_PREVIEW_DATA) != 0; }
    public boolean isForceSynchronization(){ return (options & OPTION_FORCE_SYNCHRONIZATION) != 0; }
}
