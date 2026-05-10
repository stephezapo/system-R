package org.stephezapo.system_r.core.network.sacn.packet;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * E1.31 Universe Discovery Packet (E1.31-2018 §8).
 *
 * Sources broadcast this every 10 seconds to announce which universes they are
 * transmitting. Sent to the discovery universe multicast group (239.255.250.222).
 *
 * Packet layout:
 *   [0-15]   Preamble
 *   [16-37]  Root PDU   (VECTOR_ROOT_E131_EXTENDED, CID)
 *   [38-111] Framing PDU (VECTOR_E131_EXTENDED_DISCOVERY, source name, reserved)
 *   [112+]   Discovery Layer (VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST, page, last, universes)
 */
public class SacnDiscoveryPacket extends SacnPacket
{
    // Discovery packets are sent every 10 seconds per the spec
    public static final int DISCOVERY_INTERVAL_SECONDS = 10;

    private final String     sourceName;
    private final int        page;
    private final int        lastPage;
    private final List<Integer> universes;

    public SacnDiscoveryPacket(byte[] cid, String sourceName, int page, int lastPage, List<Integer> universes)
    {
        super(cid);
        this.sourceName = sourceName;
        this.page       = page & 0xFF;
        this.lastPage   = lastPage & 0xFF;
        this.universes  = List.copyOf(universes);
    }

    @Override
    public byte[] toBytes()
    {
        int universeCount = universes.size();

        // Discovery Layer: 2 (flags+len) + 4 (vec) + 1 (page) + 1 (last) + universeCount*2
        int discoveryLen = 8 + universeCount * 2;

        // Framing PDU: 2 (flags+len) + 4 (vec) + 64 (name) + 4 (reserved) + discoveryLen
        int framingLen  = 74 + discoveryLen;

        // Root PDU: 2 (flags+len) + 4 (vec) + 16 (CID) + framingLen
        int rootLen     = 22 + framingLen;

        ByteBuffer buf = ByteBuffer.allocate(16 + rootLen);

        writePreamble(buf);
        writeRootHeader(buf, rootLen, VECTOR_ROOT_E131_EXTENDED, cid);

        // Framing PDU
        buf.putShort((short) flagsLen(framingLen));
        buf.putInt(VECTOR_E131_EXTENDED_DISCOVERY);
        writeSourceName(buf, sourceName);
        buf.putInt(0); // reserved

        // Discovery Layer
        buf.putShort((short) flagsLen(discoveryLen));
        buf.putInt(VECTOR_UNIVERSE_DISCOVERY_UNIVERSE_LIST);
        buf.put((byte) page);
        buf.put((byte) lastPage);
        for (int universe : universes)
        {
            buf.putShort((short) (universe & 0xFFFF));
        }

        return buf.array();
    }

    public static SacnDiscoveryPacket fromBytes(byte[] data)
    {
        if (data.length < 120) return null;

        byte[] cid = new byte[16];
        System.arraycopy(data, 22, cid, 0, 16);

        String sourceName = readSourceName(data, 44, 64);
        // offset 108: reserved (4 bytes), skip
        // offset 112: discovery layer flags+len (2 bytes), skip
        // offset 114: discovery layer vector (4 bytes)
        if (data.length < 120) return null;
        int page     = data[118] & 0xFF;
        int lastPage = data[119] & 0xFF;

        List<Integer> universes = new ArrayList<>();
        for (int i = 120; i + 1 < data.length; i += 2)
        {
            int universe = ((data[i] & 0xFF) << 8) | (data[i + 1] & 0xFF);
            universes.add(universe);
        }

        return new SacnDiscoveryPacket(cid, sourceName, page, lastPage, universes);
    }

    /**
     * Splits a list of universes into pages of up to 512 universes each,
     * returning one SacnDiscoveryPacket per page.
     */
    public static List<SacnDiscoveryPacket> createPages(byte[] cid, String sourceName, List<Integer> allUniverses)
    {
        List<Integer> sorted = new ArrayList<>(allUniverses);
        Collections.sort(sorted);

        int pageSize  = 512;
        int pageCount = (int) Math.ceil((double) sorted.size() / pageSize);
        if (pageCount == 0) pageCount = 1;

        List<SacnDiscoveryPacket> pages = new ArrayList<>();
        for (int p = 0; p < pageCount; p++)
        {
            int from = p * pageSize;
            int to   = Math.min(from + pageSize, sorted.size());
            pages.add(new SacnDiscoveryPacket(cid, sourceName, p, pageCount - 1, sorted.subList(from, to)));
        }
        return pages;
    }

    public String      getSourceName() { return sourceName; }
    public int         getPage()       { return page; }
    public int         getLastPage()   { return lastPage; }
    public List<Integer> getUniverses() { return universes; }
}
