package org.stephezapo.system_r.core.network.sacn.packet;

import java.nio.ByteBuffer;

/**
 * Base class for all E1.31 (sACN) packets.
 *
 * Packet layout (ACN PDU structure):
 *   [0-15]  Preamble (16 bytes)
 *   [16-37] Root PDU (22 bytes)
 *   [38+]   Framing PDU (variable)
 *
 * All multi-byte integers are big-endian unless noted otherwise.
 */
public abstract class SacnPacket
{
    // Protocol constants
    public static final int SACN_PORT     = 5568;
    public static final int DISCOVERY_UNIVERSE = 64214; // 0xFADE

    // ACN Packet Identifier: "ASC-E1.17\0\0\0"
    public static final byte[] ACN_PACKET_IDENTIFIER = {
        0x41, 0x53, 0x43, 0x2D, 0x45, 0x31, 0x2E, 0x31, 0x37, 0x00, 0x00, 0x00
    };

    // Root layer vectors
    public static final int VECTOR_ROOT_E131_DATA     = 0x00000004;
    public static final int VECTOR_ROOT_E131_EXTENDED = 0x00000008;

    // Framing layer vectors
    public static final int VECTOR_E131_DATA_PACKET              = 0x00000002;
    public static final int VECTOR_E131_EXTENDED_SYNCHRONIZATION = 0x00000001;
    public static final int VECTOR_E131_EXTENDED_DISCOVERY       = 0x00000002;

    // DMP layer
    public static final int VECTOR_DMP_SET_PROPERTY = 0x02;
    public static final int ADDRESS_TYPE_DMX512     = 0xa1;

    // Discovery layer
    public static final int VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST = 0x00000001;

    // Options flags
    public static final int OPTION_PREVIEW_DATA          = 0x20;
    public static final int OPTION_STREAM_TERMINATED     = 0x40;
    public static final int OPTION_FORCE_SYNCHRONIZATION = 0x80;

    // Default source priority
    public static final int DEFAULT_PRIORITY = 100;
    public static final int MAX_PRIORITY     = 200;

    // Multicast base: 239.255.0.0
    public static final byte MULTICAST_BASE_HI = (byte) 239;
    public static final byte MULTICAST_BASE_LO = (byte) 255;

    protected final byte[] cid;

    protected SacnPacket(byte[] cid)
    {
        this.cid = cid;
    }

    public abstract byte[] toBytes();

    /** Returns true if the raw data looks like a valid E1.31 packet. */
    public static boolean isSacn(byte[] data)
    {
        if (data.length < 16) return false;
        // Check preamble size = 0x0010
        if (data[0] != 0x00 || data[1] != 0x10) return false;
        // Check ACN packet identifier
        for (int i = 0; i < ACN_PACKET_IDENTIFIER.length; i++)
        {
            if (data[4 + i] != ACN_PACKET_IDENTIFIER[i]) return false;
        }
        return true;
    }

    /** Reads the framing layer vector from a raw packet (offset 40, 4 bytes). */
    public static int readFramingVector(byte[] data)
    {
        if (data.length < 44) return -1;
        return ((data[40] & 0xFF) << 24) | ((data[41] & 0xFF) << 16)
             | ((data[42] & 0xFF) << 8)  |  (data[43] & 0xFF);
    }

    /** Reads the root layer vector from a raw packet (offset 18, 4 bytes). */
    public static int readRootVector(byte[] data)
    {
        if (data.length < 22) return -1;
        return ((data[18] & 0xFF) << 24) | ((data[19] & 0xFF) << 16)
             | ((data[20] & 0xFF) << 8)  |  (data[21] & 0xFF);
    }

    /**
     * Writes the 16-byte ACN preamble and returns the position after it.
     * The buffer must be positioned at offset 0.
     */
    protected static void writePreamble(ByteBuffer buf)
    {
        buf.putShort((short) 0x0010); // Preamble Size
        buf.putShort((short) 0x0000); // Postamble Size
        buf.put(ACN_PACKET_IDENTIFIER);
    }

    /**
     * Writes the Root PDU header (flags+length, vector, CID).
     * @param rootPduLength the total byte length of the root PDU including this 2-byte header
     */
    protected static void writeRootHeader(ByteBuffer buf, int rootPduLength, int rootVector, byte[] cid)
    {
        buf.putShort((short) flagsLen(rootPduLength));
        buf.putInt(rootVector);
        buf.put(cid);
    }

    /** Encodes a PDU length as a 2-byte flags+length value (top nibble always 0x7). */
    protected static int flagsLen(int length)
    {
        return 0x7000 | (length & 0x0FFF);
    }

    /** Writes a source name as a 64-byte null-terminated UTF-8 field. */
    protected static void writeSourceName(ByteBuffer buf, String name)
    {
        byte[] nameBytes = name.getBytes();
        byte[] field = new byte[64];
        System.arraycopy(nameBytes, 0, field, 0, Math.min(nameBytes.length, 63));
        buf.put(field);
    }

    /** Reads a null-terminated UTF-8 string from a fixed-length field. */
    protected static String readSourceName(byte[] data, int offset, int maxLen)
    {
        var sb = new StringBuilder();
        for (int i = 0; i < maxLen && offset + i < data.length; i++)
        {
            if (data[offset + i] == 0) break;
            sb.append((char) data[offset + i]);
        }
        return sb.toString();
    }

    /** Returns the multicast InetAddress bytes for a given universe (1-63999). */
    public static byte[] universeToMulticast(int universe)
    {
        return new byte[]{
            MULTICAST_BASE_HI,
            MULTICAST_BASE_LO,
            (byte) ((universe >> 8) & 0xFF),
            (byte) (universe & 0xFF)
        };
    }

    public byte[] getCid() { return cid; }
}
