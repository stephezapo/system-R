package org.stephezapo.system_r.core.network.artnet.packet;

import org.stephezapo.system_r.core.network.artnet.ArtNetOpCode;

public class ArtPollPacket extends ArtNetPacket
{
    public static final int FLAG_DIAGNOSTICS       = 0x02;
    public static final int FLAG_DIAG_UNICAST      = 0x04;
    public static final int FLAG_VLC_TRANSMISSION  = 0x10;
    public static final int FLAG_TARGET_ADDRESS    = 0x20;

    private final int flags;
    private final int diagPriority;
    private final int targetPortAddressTop;
    private final int targetPortAddressBottom;

    public ArtPollPacket()
    {
        this(0, 0x10, 0, 0);
    }

    public ArtPollPacket(int flags, int diagPriority, int targetPortAddressTop, int targetPortAddressBottom)
    {
        super(ArtNetOpCode.OpPoll.code);
        this.flags = flags;
        this.diagPriority = diagPriority;
        this.targetPortAddressTop = targetPortAddressTop;
        this.targetPortAddressBottom = targetPortAddressBottom;
    }

    @Override
    public byte[] toBytes()
    {
        // 8 (ID) + 2 (OpCode) + 2 (ProtVer) + 1 (Flags) + 1 (DiagPriority) + 2 (TargetTop) + 2 (TargetBottom) = 18
        var buf = baseBuffer(18);
        buf.put((byte) 0x00);
        buf.put((byte) PROTOCOL_VERSION);
        buf.put((byte) flags);
        buf.put((byte) diagPriority);
        buf.putShort((short) targetPortAddressTop);
        buf.putShort((short) targetPortAddressBottom);
        return buf.array();
    }

    public static ArtPollPacket fromBytes(byte[] data)
    {
        if (data.length < 18) return null;
        int flags = data[12] & 0xFF;
        int diagPriority = data[13] & 0xFF;
        int targetTop = ((data[14] & 0xFF) << 8) | (data[15] & 0xFF);
        int targetBottom = ((data[16] & 0xFF) << 8) | (data[17] & 0xFF);
        return new ArtPollPacket(flags, diagPriority, targetTop, targetBottom);
    }

    public int getFlags() { return flags; }
    public int getDiagPriority() { return diagPriority; }
    public int getTargetPortAddressTop() { return targetPortAddressTop; }
    public int getTargetPortAddressBottom() { return targetPortAddressBottom; }
}
