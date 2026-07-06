#include "system_r/core/artnet/ArtNetPacket.h"

#include <cstring>

namespace systemr::artnet
{
namespace
{
    // Art-Net multi-byte fields are big-endian *except* the op-code, which is little-endian.
    void putBE16 (std::uint8_t* p, std::uint16_t v)
    {
        p[0] = static_cast<std::uint8_t> ((v >> 8) & 0xff);
        p[1] = static_cast<std::uint8_t> (v & 0xff);
    }

    std::uint16_t getBE16 (const std::uint8_t* p)
    {
        return static_cast<std::uint16_t> ((p[0] << 8) | p[1]);
    }

    // Writes the shared header: "Art-Net\0" + little-endian op-code. Returns bytes written (10).
    template <std::size_t N>
    std::size_t putHeader (std::array<std::uint8_t, N>& p, OpCode op)
    {
        for (std::size_t i = 0; i < kId.size(); ++i)
            p[i] = kId[i];
        const auto code = static_cast<std::uint16_t> (op);
        p[8] = static_cast<std::uint8_t> (code & 0xff);        // OpCodeLo
        p[9] = static_cast<std::uint8_t> ((code >> 8) & 0xff); // OpCodeHi
        return 10;
    }

    // Copies a JUCE string into a fixed, null-padded field (never overruns; always NUL-terminated).
    void putFixedString (std::uint8_t* dest, std::size_t len, const juce::String& s)
    {
        std::memset (dest, 0, len);
        const auto utf8 = s.toRawUTF8();
        const auto n    = juce::jmin (len - 1, std::strlen (utf8));
        std::memcpy (dest, utf8, n);
    }

    juce::String readFixedString (const std::uint8_t* src, std::size_t len)
    {
        std::size_t n = 0;
        while (n < len && src[n] != 0)
            ++n;
        return juce::String::fromUTF8 (reinterpret_cast<const char*> (src), static_cast<int> (n));
    }
}

bool isArtNet (const std::uint8_t* data, std::size_t size) noexcept
{
    if (data == nullptr || size < kId.size())
        return false;
    return std::memcmp (data, kId.data(), kId.size()) == 0;
}

std::optional<OpCode> readOpCode (const std::uint8_t* data, std::size_t size) noexcept
{
    if (! isArtNet (data, size) || size < 10)
        return std::nullopt;
    return static_cast<OpCode> (data[8] | (data[9] << 8)); // little-endian
}

//==============================================================================
DmxPacket buildDmxPacket (std::uint8_t sequence,
                          std::uint8_t physical,
                          std::uint16_t portAddress,
                          const Universe& slots)
{
    DmxPacket p {};
    putHeader (p, OpCode::Dmx);

    p[10] = 0x00;                                                  // ProtVerHi
    p[11] = static_cast<std::uint8_t> (kProtocolVersion & 0xff);   // ProtVerLo
    p[12] = sequence;
    p[13] = physical;
    p[14] = static_cast<std::uint8_t> (portAddress & 0xff);        // SubUni (low byte)
    p[15] = static_cast<std::uint8_t> ((portAddress >> 8) & 0x7f); // Net (high 7 bits)
    putBE16 (&p[16], static_cast<std::uint16_t> (kUniverseSize));  // Length (always full universe)

    for (std::size_t i = 0; i < kUniverseSize; ++i)
        p[18 + i] = slots[i];

    return p;
}

std::optional<ArtDmx> parseDmx (const std::uint8_t* data, std::size_t size)
{
    if (readOpCode (data, size) != OpCode::Dmx || size < 18)
        return std::nullopt;

    ArtDmx d;
    d.sequence    = data[12];
    d.physical    = data[13];
    const int subUni = data[14];
    const int net    = data[15] & 0x7f;
    d.portAddress = static_cast<std::uint16_t> ((net << 8) | subUni);

    const std::uint16_t length = getBE16 (&data[16]);
    d.length = static_cast<std::uint16_t> (juce::jmin<std::size_t> (length, kUniverseSize));

    const std::size_t available = size - 18;
    const std::size_t copy = juce::jmin (static_cast<std::size_t> (d.length), available);
    for (std::size_t i = 0; i < copy; ++i)
        d.slots[i] = data[18 + i];

    return d;
}

//==============================================================================
SyncPacket buildSyncPacket()
{
    SyncPacket p {};
    putHeader (p, OpCode::Sync);
    p[10] = 0x00;                                                // ProtVerHi
    p[11] = static_cast<std::uint8_t> (kProtocolVersion & 0xff); // ProtVerLo
    p[12] = 0x00;                                                // AuxHi
    p[13] = 0x00;                                                // AuxLo
    return p;
}

//==============================================================================
PollPacket buildPollPacket()
{
    PollPacket p {};
    putHeader (p, OpCode::Poll);
    p[10] = 0x00;                                                // ProtVerHi
    p[11] = static_cast<std::uint8_t> (kProtocolVersion & 0xff); // ProtVerLo
    p[12] = 0x00;                                                // Flags
    p[13] = 0x10;                                                // DiagPriority (DpLow)
    return p;
}

//==============================================================================
PollReplyPacket buildPollReply (const PollReplyConfig& cfg)
{
    PollReplyPacket p {};
    putHeader (p, OpCode::PollReply);

    for (std::size_t i = 0; i < 4; ++i)
        p[10 + i] = cfg.ip[i];
    putBE16 (&p[14], static_cast<std::uint16_t> (kPort)); // Port (always 6454)

    p[16] = 0x00;                       // VersionInfoH
    p[17] = 0x01;                       // VersionInfoL
    p[18] = cfg.netSwitch;              // NetSwitch (upper Net bits of the ports)
    p[19] = 0x00;                       // SubSwitch
    // [20..21] Oem, [22] UbeaVersion left zero.
    p[23] = 0xc0;                       // Status1: indicator normal, port-address from front panel
    // [24..25] EstaMan left zero.

    putFixedString (&p[26], 18, cfg.shortName);
    putFixedString (&p[44], 64, cfg.longName);
    putFixedString (&p[108], 64, cfg.nodeReport);

    const int numPorts = juce::jlimit (0, 4, cfg.numPorts);
    putBE16 (&p[172], static_cast<std::uint16_t> (numPorts)); // NumPorts

    for (int i = 0; i < numPorts; ++i)
    {
        p[174 + static_cast<std::size_t> (i)] = 0x80;                            // PortType: output
        p[182 + static_cast<std::size_t> (i)] = 0x80;                            // GoodOutputA: data transmitted
        p[190 + static_cast<std::size_t> (i)] = static_cast<std::uint8_t> (cfg.swOut[static_cast<std::size_t> (i)] & 0x0f); // SwOut
    }

    p[194] = 0x00;                      // AcnPriority
    p[200] = 0x00;                      // Style: StNode
    p[211] = cfg.bindIndex;             // BindIndex
    p[212] = 0x08;                      // Status2: supports 15-bit Port-Address

    return p;
}

std::optional<ArtPollReply> parsePollReply (const std::uint8_t* data, std::size_t size)
{
    if (readOpCode (data, size) != OpCode::PollReply || size < kPollReplyPacketSize)
        return std::nullopt;

    ArtPollReply r;
    for (std::size_t i = 0; i < 4; ++i)
        r.ip[i] = data[10 + i];
    r.netSwitch = data[18];
    r.shortName = readFixedString (&data[26], 18);
    r.longName  = readFixedString (&data[44], 64);
    r.numPorts  = getBE16 (&data[172]);
    for (std::size_t i = 0; i < 4; ++i)
        r.swOut[i] = data[190 + i];
    r.bindIndex = data[211];
    return r;
}
}
