#include "system_r/core/sacn/E131Packet.h"

#include <cstring>

namespace systemr::e131
{
namespace
{
    // Big-endian writers — all E1.31 multi-byte fields are network byte order.
    void put16 (DataPacket& p, std::size_t offset, std::uint16_t v)
    {
        p[offset]     = static_cast<std::uint8_t> ((v >> 8) & 0xff);
        p[offset + 1] = static_cast<std::uint8_t> (v & 0xff);
    }

    void put32 (DataPacket& p, std::size_t offset, std::uint32_t v)
    {
        p[offset]     = static_cast<std::uint8_t> ((v >> 24) & 0xff);
        p[offset + 1] = static_cast<std::uint8_t> ((v >> 16) & 0xff);
        p[offset + 2] = static_cast<std::uint8_t> ((v >> 8) & 0xff);
        p[offset + 3] = static_cast<std::uint8_t> (v & 0xff);
    }

    // Flags (top nibble 0x7) + PDU length (low 12 bits) = bytes from this field to end.
    std::uint16_t flagsAndLength (std::size_t pduBytes)
    {
        return static_cast<std::uint16_t> (0x7000 | (pduBytes & 0x0fff));
    }

    constexpr std::uint32_t kVectorRootData   = 0x00000004; // VECTOR_ROOT_E131_DATA
    constexpr std::uint32_t kVectorFramingData = 0x00000002; // VECTOR_E131_DATA_PACKET
    constexpr std::uint8_t  kVectorDmpSetProp  = 0x02;       // VECTOR_DMP_SET_PROPERTY

    constexpr std::uint16_t kPropertyValueCount = 0x0201;    // 1 start code + 512 slots
}

juce::String multicastAddressFor (int universe)
{
    const int hi = (universe >> 8) & 0xff;
    const int lo = universe & 0xff;
    return "239.255." + juce::String (hi) + "." + juce::String (lo);
}

DataPacket buildDataPacket (const juce::Uuid& cid,
                            const juce::String& sourceName,
                            std::uint8_t priority,
                            std::uint16_t syncUniverse,
                            std::uint8_t sequence,
                            std::uint8_t options,
                            std::uint16_t universe,
                            const Universe& slots)
{
    DataPacket p {};

    // ---- Root Layer (offset 0..37) ----
    put16 (p, 0, 0x0010);                        // Preamble size
    put16 (p, 2, 0x0000);                        // Post-amble size
    static constexpr std::uint8_t acnId[12] =    // "ASC-E1.17\0\0\0"
        { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 };
    for (std::size_t i = 0; i < 12; ++i)
        p[4 + i] = acnId[i];
    put16 (p, 16, flagsAndLength (kDataPacketSize - 16)); // Flags & length (622)
    put32 (p, 18, kVectorRootData);              // Root vector
    {
        const auto* raw = cid.getRawData();      // 16-byte CID
        for (std::size_t i = 0; i < 16; ++i)
            p[22 + i] = raw[i];
    }

    // ---- E1.31 Framing Layer (offset 38..114) ----
    put16 (p, 38, flagsAndLength (kDataPacketSize - 38)); // Flags & length (600)
    put32 (p, 40, kVectorFramingData);           // Framing vector
    {
        const auto utf8 = sourceName.toRawUTF8();
        const auto len  = juce::jmin<std::size_t> (63, std::strlen (utf8));
        for (std::size_t i = 0; i < len; ++i)
            p[44 + i] = static_cast<std::uint8_t> (utf8[i]);
        // remaining bytes stay zero (null-terminated / padded)
    }
    p[108] = priority;                           // Priority
    put16 (p, 109, syncUniverse);                // Synchronization address
    p[111] = sequence;                           // Sequence number
    p[112] = options;                            // Options
    put16 (p, 113, universe);                    // Universe

    // ---- DMP Layer (offset 115..637) ----
    put16 (p, 115, flagsAndLength (kDataPacketSize - 115)); // Flags & length (523)
    p[117] = kVectorDmpSetProp;                  // DMP vector
    p[118] = 0xa1;                               // Address type & data type
    put16 (p, 119, 0x0000);                      // First property address
    put16 (p, 121, 0x0001);                      // Address increment
    put16 (p, 123, kPropertyValueCount);         // Property value count (513)
    p[125] = 0x00;                               // DMX512 START code
    for (int i = 0; i < kUniverseSize; ++i)
        p[126 + static_cast<std::size_t> (i)] = slots[static_cast<std::size_t> (i)];

    return p;
}

namespace
{
    std::uint16_t read16 (const std::uint8_t* p)
    {
        return static_cast<std::uint16_t> ((p[0] << 8) | p[1]);
    }

    std::uint32_t read32 (const std::uint8_t* p)
    {
        return (static_cast<std::uint32_t> (p[0]) << 24)
             | (static_cast<std::uint32_t> (p[1]) << 16)
             | (static_cast<std::uint32_t> (p[2]) << 8)
             |  static_cast<std::uint32_t> (p[3]);
    }

    // Reads a NUL-terminated string from a fixed-length field.
    juce::String readFixedString (const std::uint8_t* src, std::size_t len)
    {
        std::size_t n = 0;
        while (n < len && src[n] != 0)
            ++n;
        return juce::String::fromUTF8 (reinterpret_cast<const char*> (src), static_cast<int> (n));
    }
}

std::optional<DataPacketInfo> parseDataPacket (const std::uint8_t* data, std::size_t size)
{
    // Smallest valid data packet: full header (126) + start code + at least one slot.
    if (data == nullptr || size < 126)
        return std::nullopt;

    // Root layer: preamble 0x0010, post-amble 0x0000, ACN packet identifier.
    if (read16 (&data[0]) != 0x0010 || read16 (&data[2]) != 0x0000)
        return std::nullopt;

    static constexpr std::uint8_t acnId[12] =
        { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 };
    if (std::memcmp (&data[4], acnId, 12) != 0)
        return std::nullopt;

    // Vectors identify a data packet across the three layers.
    if (read32 (&data[18]) != kVectorRootData)     return std::nullopt;
    if (read32 (&data[40]) != kVectorFramingData)  return std::nullopt;
    if (data[117] != kVectorDmpSetProp)            return std::nullopt;
    if (data[125] != 0x00)                         return std::nullopt; // DMX start code only

    DataPacketInfo info;
    info.cid          = juce::Uuid (&data[22]);
    info.sourceName   = readFixedString (&data[44], 64);
    info.priority     = data[108];
    info.syncUniverse = read16 (&data[109]);
    info.sequence     = data[111];
    info.options      = data[112];
    info.universe     = read16 (&data[113]);

    // DMP property value count = 1 (start code) + slot count.
    const std::uint16_t propCount = read16 (&data[123]);
    const std::uint16_t slotCount = propCount > 0 ? static_cast<std::uint16_t> (propCount - 1) : 0;
    info.slotCount = static_cast<std::uint16_t> (juce::jmin<int> (slotCount, kUniverseSize));

    const std::size_t available = size - 126;
    const std::size_t copy = juce::jmin (static_cast<std::size_t> (info.slotCount), available);
    for (std::size_t i = 0; i < copy; ++i)
        info.slots[i] = data[126 + i];

    return info;
}
}
