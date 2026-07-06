#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/Universe.h"

#include <array>
#include <cstdint>
#include <optional>

namespace systemr::e131
{
/** UDP port for streaming ACN (ANSI E1.31-2018 §4.1). */
inline constexpr int kPort = 5568;

/** Framing-layer Options bits (E1.31 §6.2.6). */
inline constexpr std::uint8_t kOptionPreview        = 0x80;
inline constexpr std::uint8_t kOptionStreamTerminated = 0x40;
inline constexpr std::uint8_t kOptionForceSync      = 0x20;

/** Default data priority (E1.31 §6.2.3). */
inline constexpr std::uint8_t kDefaultPriority = 100;

/** Size of a full E1.31 data packet carrying 512 slots + start code. */
inline constexpr std::size_t kDataPacketSize = 638;

using DataPacket = std::array<std::uint8_t, kDataPacketSize>;

/** IPv4 multicast group for a universe (E1.31 Appendix A): 239.255.<hi>.<lo>. */
[[nodiscard]] juce::String multicastAddressFor (int universe);

/** Builds a complete E1.31-2018 E1.31 Data Packet (root + framing + DMP layers)
    for a full 512-slot universe with DMX start code 0x00.

    @param cid          this source's persistent component identifier (16 bytes)
    @param sourceName   human-readable source name (truncated/padded to 64 bytes)
    @param priority     0..200, higher wins on the receiver
    @param syncUniverse universe of the sync packet, or 0 for unsynchronised
    @param sequence     per-universe sequence number, incremented each packet
    @param options      framing options bitfield (Preview/Terminated/Force-sync)
    @param universe     1..63999
    @param slots        the 512 DMX slot values
*/
[[nodiscard]] DataPacket buildDataPacket (const juce::Uuid& cid,
                                          const juce::String& sourceName,
                                          std::uint8_t priority,
                                          std::uint16_t syncUniverse,
                                          std::uint8_t sequence,
                                          std::uint8_t options,
                                          std::uint16_t universe,
                                          const Universe& slots);

/** Decoded contents of a received E1.31 Data Packet. */
struct DataPacketInfo
{
    juce::Uuid    cid;
    juce::String  sourceName;
    std::uint8_t  priority = 0;
    std::uint16_t syncUniverse = 0;
    std::uint8_t  sequence = 0;
    std::uint8_t  options = 0;
    std::uint16_t universe = 0;
    std::uint16_t slotCount = 0; // number of DMX slots (excludes start code)
    Universe      slots {};      // zero-padded to a full universe
};

/** Parses an E1.31 Data Packet, validating the ACN/root/framing/DMP layers and the
    DMX (0x00) start code. Returns nullopt for anything that isn't a well-formed
    E1.31 data packet (other start codes, discovery/sync packets, malformed input). */
[[nodiscard]] std::optional<DataPacketInfo> parseDataPacket (const std::uint8_t* data, std::size_t size);
}
