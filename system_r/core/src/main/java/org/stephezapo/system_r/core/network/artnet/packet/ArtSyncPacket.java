package org.stephezapo.system_r.core.network.artnet.packet;

import org.stephezapo.system_r.core.network.artnet.ArtNetOpCode;

public class ArtSyncPacket extends ArtNetPacket
{
    public ArtSyncPacket()
    {
        super(ArtNetOpCode.OpSync.code);
    }

    @Override
    public byte[] toBytes()
    {
        // 8 (ID) + 2 (OpCode) + 2 (ProtVer) + 2 (Aux) = 14 bytes
        var buf = baseBuffer(14);
        buf.put((byte) 0x00); // ProtVer Hi
        buf.put((byte) PROTOCOL_VERSION);
        buf.put((byte) 0x00); // AuxHi
        buf.put((byte) 0x00); // AuxLo
        return buf.array();
    }

    public static ArtSyncPacket fromBytes(byte[] data)
    {
        if (data.length < 14) return null;
        return new ArtSyncPacket();
    }
}
