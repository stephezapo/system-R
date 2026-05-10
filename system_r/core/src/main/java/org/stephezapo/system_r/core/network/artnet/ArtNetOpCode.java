package org.stephezapo.system_r.core.network.artnet;

public enum ArtNetOpCode
{
    OpPoll(0x2000),
    OpPollReply(0x2100),
    OpDiagData(0x2300),
    OpCommand(0x2400),
    OpDmx(0x5000),
    OpNzs(0x5100),
    OpSync(0x5200),
    OpAddress(0x6000),
    OpInput(0x7000),
    OpTodRequest(0x8000),
    OpTodData(0x8100),
    OpTodControl(0x8200),
    OpRdm(0x8300),
    OpRdmSub(0x8400),
    OpTimeCode(0x9700),
    OpTimeSync(0x9800),
    OpTrigger(0x9900),
    OpDirectory(0x9A00),
    OpDirectoryReply(0x9B00),
    OpIpProg(0xF800),
    OpIpProgReply(0xF900),
    OpFirmwareMaster(0xF200),
    OpFirmwareReply(0xF300),
    OpFileTnMaster(0xF400),
    OpFileFnMaster(0xF500),
    OpFileFnReply(0xF600);

    public final int code;

    ArtNetOpCode(int code)
    {
        this.code = code;
    }

    public static ArtNetOpCode fromCode(int code)
    {
        for (ArtNetOpCode op : values())
        {
            if (op.code == code) return op;
        }
        return null;
    }
}
