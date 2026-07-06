#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/Universe.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace systemr::artnet
{
/** UDP port for Art-Net (Art-Net 4, Artistic Licence). */
inline constexpr int kPort = 6454;

/** Art-Net protocol revision this console speaks (ArtDmx ProtVer = 14). */
inline constexpr std::uint16_t kProtocolVersion = 14;

/** Packet identifier "Art-Net\0" that opens every Art-Net datagram. */
inline constexpr std::array<std::uint8_t, 8> kId {
    'A', 'r', 't', '-', 'N', 'e', 't', 0
};

/** Op-codes, stored/transmitted little-endian in the wire header. */
enum class OpCode : std::uint16_t
{
    Poll      = 0x2000,
    PollReply = 0x2100,
    Dmx       = 0x5000,
    Sync      = 0x5200,
    Address   = 0x6000,
};

/** Composes a 15-bit Port-Address from its Net(7)/Sub-Net(4)/Universe(4) fields. */
[[nodiscard]] constexpr std::uint16_t makePortAddress (int net, int subNet, int universe) noexcept
{
    return static_cast<std::uint16_t> (((net & 0x7f) << 8)
                                       | ((subNet & 0x0f) << 4)
                                       | (universe & 0x0f));
}

/** Formats a Port-Address as "Net:Sub:Universe" for display. */
[[nodiscard]] inline juce::String formatPortAddress (std::uint16_t portAddress)
{
    const int net = (portAddress >> 8) & 0x7f;
    const int sub = (portAddress >> 4) & 0x0f;
    const int uni = portAddress & 0x0f;
    return juce::String (net) + ":" + juce::String (sub) + ":" + juce::String (uni);
}

/** True if the buffer opens with the Art-Net packet identifier. */
[[nodiscard]] bool isArtNet (const std::uint8_t* data, std::size_t size) noexcept;

/** Reads the little-endian op-code, or nullopt if the buffer is too short / not Art-Net. */
[[nodiscard]] std::optional<OpCode> readOpCode (const std::uint8_t* data, std::size_t size) noexcept;

//==============================================================================
// ArtDmx (OpDmx) — carries one universe of channel data.

/** Fixed on-wire size of a full-universe ArtDmx packet (18 header + 512 slots). */
inline constexpr std::size_t kDmxPacketSize = 18 + kUniverseSize;

using DmxPacket = std::array<std::uint8_t, kDmxPacketSize>;

/** Builds a full 512-slot ArtDmx packet for a Port-Address.
    @param sequence     0 disables ordering; 1..255 lets receivers drop out-of-order frames
    @param physical     informative physical input port (0 for a controller)
    @param portAddress  15-bit Net/Sub-Net/Universe address
    @param slots        the 512 DMX slot values
*/
[[nodiscard]] DmxPacket buildDmxPacket (std::uint8_t sequence,
                                        std::uint8_t physical,
                                        std::uint16_t portAddress,
                                        const Universe& slots);

/** Decoded ArtDmx contents. */
struct ArtDmx
{
    std::uint8_t  sequence = 0;
    std::uint8_t  physical = 0;
    std::uint16_t portAddress = 0; // 15-bit
    std::uint16_t length = 0;      // number of valid slots (<= 512)
    Universe      slots {};        // zero-padded to a full universe
};

/** Parses an ArtDmx packet, or nullopt if it is malformed / a different op-code. */
[[nodiscard]] std::optional<ArtDmx> parseDmx (const std::uint8_t* data, std::size_t size);

//==============================================================================
// ArtSync (OpSync) — releases buffered ArtDmx frames simultaneously.

inline constexpr std::size_t kSyncPacketSize = 14;
using SyncPacket = std::array<std::uint8_t, kSyncPacketSize>;

[[nodiscard]] SyncPacket buildSyncPacket();

//==============================================================================
// ArtPoll (OpPoll) — discovery request.

inline constexpr std::size_t kPollPacketSize = 14;
using PollPacket = std::array<std::uint8_t, kPollPacketSize>;

[[nodiscard]] PollPacket buildPollPacket();

//==============================================================================
// ArtPollReply (OpPollReply) — discovery advertisement (no ProtVer field).

inline constexpr std::size_t kPollReplyPacketSize = 239;
using PollReplyPacket = std::array<std::uint8_t, kPollReplyPacketSize>;

/** How this console advertises itself in an ArtPollReply.
    Up to four output Port-Addresses can be reported per reply, per the spec's
    four-port node model; more universes are advertised across BindIndex'd replies. */
struct PollReplyConfig
{
    std::array<std::uint8_t, 4> ip { 0, 0, 0, 0 };
    juce::String shortName { "system-R" };
    juce::String longName  { "system-R lighting console" };
    juce::String nodeReport;
    int numPorts = 0;                          // 0..4 advertised output ports
    std::array<std::uint16_t, 4> swOut { 0, 0, 0, 0 }; // per-port Sub-Net/Universe
    std::uint8_t netSwitch = 0;                // upper Net of the reported ports
    std::uint8_t bindIndex = 1;                // 1-based; distinguishes multi-reply nodes
};

[[nodiscard]] PollReplyPacket buildPollReply (const PollReplyConfig& cfg);

/** Key fields decoded from an ArtPollReply (enough for a node list). */
struct ArtPollReply
{
    std::array<std::uint8_t, 4> ip { 0, 0, 0, 0 };
    juce::String shortName;
    juce::String longName;
    int numPorts = 0;
    std::array<std::uint16_t, 4> swOut { 0, 0, 0, 0 };
    std::uint8_t netSwitch = 0;
    std::uint8_t bindIndex = 0;
};

[[nodiscard]] std::optional<ArtPollReply> parsePollReply (const std::uint8_t* data, std::size_t size);
}
