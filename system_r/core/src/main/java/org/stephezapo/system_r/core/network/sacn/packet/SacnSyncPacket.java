package org.stephezapo.system_r.core.network.sacn.packet;

import java.nio.ByteBuffer;

/**
 * E1.31 Synchronization Packet (E1.31-2018 §6.3.3).
 *
 * Sent to the synchronization address multicast group to trigger simultaneous
 * output of all universes that declared that sync address.
 *
 * Packet layout:
 *   [0-15]  Preamble
 *   [16-37] Root PDU (VECTOR_ROOT_E131_EXTENDED, CID)
 *   [38-44] Framing PDU (VECTOR_E131_EXTENDED_SYNCHRONIZATION, sequence, reserved)
 */
public class SacnSyncPacket extends SacnPacket
{
    private final int sequence;
    private final int syncAddress;

    public SacnSyncPacket(byte[] cid, int sequence, int syncAddress)
    {
        super(cid);
        this.sequence    = sequence & 0xFF;
        this.syncAddress = syncAddress & 0xFFFF;
    }

    @Override
    public byte[] toBytes()
    {
        // Framing PDU: 2 (flags+len) + 4 (vec) + 1 (seq) + 2 (reserved) = 9
        int framingLen = 9;
        // Root PDU: 2 (flags+len) + 4 (vec) + 16 (CID) + framingLen = 31
        int rootLen   = 22 + framingLen;

        ByteBuffer buf = ByteBuffer.allocate(16 + rootLen);

        writePreamble(buf);
        writeRootHeader(buf, rootLen, VECTOR_ROOT_E131_EXTENDED, cid);

        // Framing PDU
        buf.putShort((short) flagsLen(framingLen));
        buf.putInt(VECTOR_E131_EXTENDED_SYNCHRONIZATION);
        buf.put((byte) sequence);
        buf.putShort((short) 0x0000); // reserved

        return buf.array();
    }

    public static SacnSyncPacket fromBytes(byte[] data)
    {
        if (data.length < 47) return null;
        byte[] cid = new byte[16];
        System.arraycopy(data, 22, cid, 0, 16);
        int sequence    = data[44] & 0xFF;
        int syncAddress = 0; // not carried in the packet body, derived from the multicast group
        return new SacnSyncPacket(cid, sequence, syncAddress);
    }

    public int getSequence()    { return sequence; }
    public int getSyncAddress() { return syncAddress; }
}
