package org.stephezapo.system_r.core.network.artnet.packet;

import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import org.stephezapo.system_r.core.network.artnet.ArtNetOpCode;

/**
 * Art-Net 4 ArtPollReply packet — 239 bytes, no ProtVer field.
 * Byte offsets follow the Art-Net 4 specification table.
 */
public class ArtPollReplyPacket extends ArtNetPacket
{
    public static final int PACKET_SIZE = 239;

    // Status1 flags
    public static final int STATUS1_UBEA_PRESENT               = 0x01;
    public static final int STATUS1_RDM_CAPABLE                = 0x02;
    public static final int STATUS1_ROM_BOOTED                 = 0x04;
    public static final int STATUS1_PORT_ADDRESS_AUTHORITY_NET = 0x20;
    public static final int STATUS1_INDICATOR_LOCATE           = 0x40;
    public static final int STATUS1_INDICATOR_MUTE             = 0x80;
    public static final int STATUS1_INDICATOR_NORMAL           = 0xC0;

    // Status2 flags
    public static final int STATUS2_WEB_BROWSER       = 0x01;
    public static final int STATUS2_DHCP_CONFIGURED   = 0x02;
    public static final int STATUS2_DHCP_CAPABLE      = 0x04;
    public static final int STATUS2_15BIT_PORT        = 0x08;
    public static final int STATUS2_SACN_SUPPORT      = 0x10;
    public static final int STATUS2_SQUAWKING         = 0x20;
    public static final int STATUS2_OUTPUT_SWITCHING  = 0x40;

    // Port type flags
    public static final int PORT_TYPE_OUTPUT = 0x80;
    public static final int PORT_TYPE_INPUT  = 0x40;

    // Style codes
    public static final int STYLE_NODE      = 0x00;
    public static final int STYLE_CONTROLLER = 0x01;
    public static final int STYLE_MEDIA     = 0x02;
    public static final int STYLE_ROUTE     = 0x03;
    public static final int STYLE_BACKUP    = 0x04;
    public static final int STYLE_CONFIG    = 0x05;
    public static final int STYLE_VISUAL    = 0x06;

    private final byte[] ipAddress     = new byte[4];
    private int versionInfoH;
    private int versionInfoL;
    private int netSwitch;
    private int subSwitch;
    private int oemHi;
    private int oemLo;
    private int ubeaVersion;
    private int status1;
    private int estaManLo;
    private int estaManHi;
    private String shortName  = "";
    private String longName   = "";
    private String nodeReport = "";
    private int numPorts;
    private final int[] portTypes   = new int[4];
    private final int[] goodInput   = new int[4];
    private final int[] goodOutputA = new int[4];
    private final int[] swIn        = new int[4];
    private final int[] swOut       = new int[4];
    private int acnPriority;
    private int swMacro;
    private int swRemote;
    private int style;
    private final byte[] mac            = new byte[6];
    private final byte[] bindIp         = new byte[4];
    private int bindIndex;
    private int status2;
    private final int[] goodOutputB     = new int[4];
    private int status3;
    private final byte[] defaultRespUID = new byte[6];

    public ArtPollReplyPacket()
    {
        super(ArtNetOpCode.OpPollReply.code);
    }

    public static ArtPollReplyPacket fromBytes(byte[] data)
    {
        if (data.length < PACKET_SIZE) return null;

        var p = new ArtPollReplyPacket();
        int i = 10; // skip ID (8) + OpCode (2)

        System.arraycopy(data, i, p.ipAddress, 0, 4); i += 4;
        i += 2; // port — always 6454, ignore
        p.versionInfoH = data[i++] & 0xFF;
        p.versionInfoL = data[i++] & 0xFF;
        p.netSwitch    = data[i++] & 0xFF;
        p.subSwitch    = data[i++] & 0xFF;
        p.oemHi        = data[i++] & 0xFF;
        p.oemLo        = data[i++] & 0xFF;
        p.ubeaVersion  = data[i++] & 0xFF;
        p.status1      = data[i++] & 0xFF;
        p.estaManLo    = data[i++] & 0xFF; // little-endian: Lo first
        p.estaManHi    = data[i++] & 0xFF;
        p.shortName    = readString(data, i, 18);  i += 18;
        p.longName     = readString(data, i, 64);  i += 64;
        p.nodeReport   = readString(data, i, 64);  i += 64;
        p.numPorts     = ((data[i] & 0xFF) << 8) | (data[i + 1] & 0xFF); i += 2;
        for (int k = 0; k < 4; k++) p.portTypes[k]   = data[i++] & 0xFF;
        for (int k = 0; k < 4; k++) p.goodInput[k]   = data[i++] & 0xFF;
        for (int k = 0; k < 4; k++) p.goodOutputA[k] = data[i++] & 0xFF;
        for (int k = 0; k < 4; k++) p.swIn[k]        = data[i++] & 0xFF;
        for (int k = 0; k < 4; k++) p.swOut[k]       = data[i++] & 0xFF;
        p.acnPriority  = data[i++] & 0xFF;
        p.swMacro      = data[i++] & 0xFF;
        p.swRemote     = data[i++] & 0xFF;
        i += 3; // spare
        p.style        = data[i++] & 0xFF;
        System.arraycopy(data, i, p.mac, 0, 6);    i += 6;
        System.arraycopy(data, i, p.bindIp, 0, 4); i += 4;
        p.bindIndex    = data[i++] & 0xFF;
        p.status2      = data[i++] & 0xFF;
        for (int k = 0; k < 4; k++) p.goodOutputB[k] = data[i++] & 0xFF;
        p.status3      = data[i++] & 0xFF;
        System.arraycopy(data, i, p.defaultRespUID, 0, 6);
        return p;
    }

    @Override
    public byte[] toBytes()
    {
        ByteBuffer buf = ByteBuffer.allocate(PACKET_SIZE);
        buf.order(ByteOrder.BIG_ENDIAN);
        buf.put(ARTNET_ID);
        buf.put((byte) (ArtNetOpCode.OpPollReply.code & 0xFF));
        buf.put((byte) ((ArtNetOpCode.OpPollReply.code >> 8) & 0xFF));
        buf.put(ipAddress);
        buf.putShort((short) ARTNET_PORT);
        buf.put((byte) versionInfoH);
        buf.put((byte) versionInfoL);
        buf.put((byte) netSwitch);
        buf.put((byte) subSwitch);
        buf.put((byte) oemHi);
        buf.put((byte) oemLo);
        buf.put((byte) ubeaVersion);
        buf.put((byte) status1);
        buf.put((byte) estaManLo); // little-endian: Lo first
        buf.put((byte) estaManHi);
        buf.put(fixedBytes(shortName, 18));
        buf.put(fixedBytes(longName, 64));
        buf.put(fixedBytes(nodeReport, 64));
        buf.put((byte) ((numPorts >> 8) & 0xFF));
        buf.put((byte) (numPorts & 0xFF));
        for (int t : portTypes)   buf.put((byte) t);
        for (int g : goodInput)   buf.put((byte) g);
        for (int g : goodOutputA) buf.put((byte) g);
        for (int s : swIn)        buf.put((byte) s);
        for (int s : swOut)       buf.put((byte) s);
        buf.put((byte) acnPriority);
        buf.put((byte) swMacro);
        buf.put((byte) swRemote);
        buf.put(new byte[3]); // spare
        buf.put((byte) style);
        buf.put(mac);
        buf.put(bindIp);
        buf.put((byte) bindIndex);
        buf.put((byte) status2);
        for (int g : goodOutputB) buf.put((byte) g);
        buf.put((byte) status3);
        buf.put(defaultRespUID);
        // remaining bytes to reach 239 (UserHi, UserLo, RefillPort3 fields, padding)
        int remaining = PACKET_SIZE - buf.position();
        if (remaining > 0) buf.put(new byte[remaining]);
        return buf.array();
    }

    private static String readString(byte[] data, int offset, int maxLen)
    {
        var sb = new StringBuilder();
        for (int i = 0; i < maxLen && offset + i < data.length; i++)
        {
            if (data[offset + i] == 0) break;
            sb.append((char) data[offset + i]);
        }
        return sb.toString();
    }

    private static byte[] fixedBytes(String s, int len)
    {
        byte[] result = new byte[len];
        byte[] src = s.getBytes();
        System.arraycopy(src, 0, result, 0, Math.min(src.length, len - 1));
        return result;
    }

    public byte[] getIpAddress() { return Arrays.copyOf(ipAddress, 4); }
    public void setIpAddress(InetAddress addr) { System.arraycopy(addr.getAddress(), 0, ipAddress, 0, 4); }

    public int getNetSwitch() { return netSwitch; }
    public void setNetSwitch(int netSwitch) { this.netSwitch = netSwitch & 0x7F; }

    public int getSubSwitch() { return subSwitch; }
    public void setSubSwitch(int subSwitch) { this.subSwitch = subSwitch & 0x0F; }

    public String getShortName() { return shortName; }
    public void setShortName(String shortName) { this.shortName = shortName; }

    public String getLongName() { return longName; }
    public void setLongName(String longName) { this.longName = longName; }

    public String getNodeReport() { return nodeReport; }
    public void setNodeReport(String nodeReport) { this.nodeReport = nodeReport; }

    public int getStatus1() { return status1; }
    public void setStatus1(int status1) { this.status1 = status1; }

    public int getStatus2() { return status2; }
    public void setStatus2(int status2) { this.status2 = status2; }

    public int getStatus3() { return status3; }
    public void setStatus3(int status3) { this.status3 = status3; }

    public int getNumPorts() { return numPorts; }
    public void setNumPorts(int numPorts) { this.numPorts = numPorts; }

    public int[] getPortTypes() { return portTypes; }
    public int[] getSwIn() { return swIn; }
    public int[] getSwOut() { return swOut; }
    public int[] getGoodInput() { return goodInput; }
    public int[] getGoodOutputA() { return goodOutputA; }
    public int[] getGoodOutputB() { return goodOutputB; }

    public int getStyle() { return style; }
    public void setStyle(int style) { this.style = style; }

    public byte[] getMac() { return Arrays.copyOf(mac, 6); }
    public void setMac(byte[] mac) { System.arraycopy(mac, 0, this.mac, 0, Math.min(mac.length, 6)); }

    public byte[] getBindIp() { return Arrays.copyOf(bindIp, 4); }
    public void setBindIp(byte[] bindIp) { System.arraycopy(bindIp, 0, this.bindIp, 0, Math.min(bindIp.length, 4)); }

    public int getBindIndex() { return bindIndex; }
    public void setBindIndex(int bindIndex) { this.bindIndex = bindIndex; }

    public int getAcnPriority() { return acnPriority; }
    public void setAcnPriority(int acnPriority) { this.acnPriority = acnPriority; }

    public int getVersionInfoH() { return versionInfoH; }
    public void setVersionInfoH(int v) { this.versionInfoH = v; }

    public int getVersionInfoL() { return versionInfoL; }
    public void setVersionInfoL(int v) { this.versionInfoL = v; }

    public int getOemHi() { return oemHi; }
    public void setOemHi(int oemHi) { this.oemHi = oemHi; }

    public int getOemLo() { return oemLo; }
    public void setOemLo(int oemLo) { this.oemLo = oemLo; }

    public int getEstaManHi() { return estaManHi; }
    public void setEstaManHi(int v) { this.estaManHi = v; }

    public int getEstaManLo() { return estaManLo; }
    public void setEstaManLo(int v) { this.estaManLo = v; }

    public byte[] getDefaultRespUID() { return Arrays.copyOf(defaultRespUID, 6); }
}
