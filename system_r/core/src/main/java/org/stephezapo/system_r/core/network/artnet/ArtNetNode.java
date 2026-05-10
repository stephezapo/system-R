package org.stephezapo.system_r.core.network.artnet;

import java.net.InetAddress;
import org.stephezapo.system_r.core.network.artnet.packet.ArtPollReplyPacket;

public class ArtNetNode
{
    private final InetAddress address;
    private final ArtPollReplyPacket replyPacket;
    private volatile long lastSeen;

    public ArtNetNode(InetAddress address, ArtPollReplyPacket replyPacket)
    {
        this.address = address;
        this.replyPacket = replyPacket;
        this.lastSeen = System.currentTimeMillis();
    }

    public void updateLastSeen()
    {
        this.lastSeen = System.currentTimeMillis();
    }

    public InetAddress getAddress() { return address; }
    public ArtPollReplyPacket getReplyPacket() { return replyPacket; }
    public long getLastSeen() { return lastSeen; }

    public String getShortName() { return replyPacket.getShortName(); }
    public String getLongName() { return replyPacket.getLongName(); }
    public int getNetSwitch() { return replyPacket.getNetSwitch(); }
    public int getSubSwitch() { return replyPacket.getSubSwitch(); }
    public int[] getSwOut() { return replyPacket.getSwOut(); }
    public int[] getSwIn() { return replyPacket.getSwIn(); }

    @Override
    public String toString()
    {
        return "ArtNetNode{ip=" + address.getHostAddress() +
               ", name='" + getShortName() + "'" +
               ", net=" + getNetSwitch() + ", sub=" + getSubSwitch() + "}";
    }
}
