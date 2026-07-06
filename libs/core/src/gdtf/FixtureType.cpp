#include "system_r/core/gdtf/FixtureType.h"

namespace systemr::gdtf
{
DmxValue DmxValue::parse (const juce::String& s, int defaultBytes)
{
    DmxValue v;
    v.bytes = juce::jlimit (1, 4, defaultBytes);

    const auto trimmed = s.trim();
    if (trimmed.isEmpty())
        return v;

    const int slash = trimmed.indexOfChar ('/');
    if (slash < 0)
    {
        v.value = static_cast<std::uint32_t> (trimmed.getLargeIntValue());
    }
    else
    {
        v.value = static_cast<std::uint32_t> (trimmed.substring (0, slash).getLargeIntValue());
        v.bytes = juce::jlimit (1, 4, trimmed.substring (slash + 1).getIntValue());
    }
    return v;
}

std::uint32_t DmxValue::toChannelValue (int channelBytes) const
{
    channelBytes = juce::jlimit (1, 4, channelBytes);
    if (channelBytes == bytes)
        return value;

    if (channelBytes > bytes)
    {
        // Widen per GDTF: shift up and fill new low bytes by replicating the top byte.
        std::uint32_t result = value << (8 * (channelBytes - bytes));
        const std::uint32_t topByte = (value >> (8 * (bytes - 1))) & 0xff;
        for (int b = 0; b < channelBytes - bytes; ++b)
            result |= topByte << (8 * b);
        return result;
    }

    // Narrow: keep the most-significant channelBytes.
    return value >> (8 * (bytes - channelBytes));
}

int DmxMode::channelCount() const noexcept
{
    int maxOffset = 0;
    for (const auto& ch : channels)
        for (int off : ch.offsets)
            maxOffset = juce::jmax (maxOffset, off);
    return maxOffset;
}

juce::String FixtureType::id() const
{
    if (fixtureTypeId.isNotEmpty())
        return fixtureTypeId;
    return manufacturer + " / " + name;
}

const DmxMode* FixtureType::findMode (const juce::String& modeName) const
{
    for (const auto& m : modes)
        if (m.name == modeName)
            return &m;
    return nullptr;
}

std::uint32_t scaleLevel (std::uint8_t level, int bytes) noexcept
{
    bytes = juce::jlimit (1, 4, bytes);
    // Max value for the width; 8-bit stays identity, wider scales full-range.
    const std::uint64_t maxValue = (std::uint64_t { 1 } << (8 * bytes)) - 1;
    return static_cast<std::uint32_t> ((static_cast<std::uint64_t> (level) * maxValue + 127) / 255);
}

void writeChannelValue (Universe& out, int startAddress, const DmxChannel& channel, std::uint32_t value)
{
    const int n = channel.resolution();
    if (channel.isVirtual())
        return;

    for (int i = 0; i < n; ++i)
    {
        // offsets[0] is coarse (most significant); offsets are 1-based within the mode.
        const int absAddress = startAddress + channel.offsets[static_cast<std::size_t> (i)] - 1; // 1-based
        const int idx = absAddress - 1;                                                          // 0-based slot
        if (idx < 0 || idx >= kUniverseSize)
            continue;

        const auto byte = static_cast<std::uint8_t> ((value >> (8 * (n - 1 - i))) & 0xff);
        out[static_cast<std::size_t> (idx)] = byte;
    }
}
}
