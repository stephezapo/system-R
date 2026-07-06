#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/Universe.h"

#include <cstdint>
#include <vector>

namespace systemr::gdtf
{
/** A GDTF DMXValue: an integer plus its byte width, written "value/bytes" (e.g. "255/1",
    "32768/2"). Coarse byte is most-significant. */
struct DmxValue
{
    std::uint32_t value = 0;
    int bytes = 1;

    /** Parses "value/bytes"; falls back to @p defaultBytes when the width is omitted. */
    [[nodiscard]] static DmxValue parse (const juce::String& s, int defaultBytes = 1);

    /** Re-scales this value to a channel of @p channelBytes width (GDTF bit-shift rule:
        widen by replicating the coarse byte, narrow by truncation). */
    [[nodiscard]] std::uint32_t toChannelValue (int channelBytes) const;
};

/** One channel function: a DMX sub-range mapped to a physical attribute range. Enough is
    captured for later attribute→DMX resolution (Phase 4); channel sets/wheels are skipped. */
struct ChannelFunction
{
    juce::String attribute;
    juce::String name;
    DmxValue     dmxFrom { 0, 1 };
    DmxValue     defaultValue { 0, 1 };
    double       physicalFrom = 0.0;
    double       physicalTo = 1.0;
};

/** One DMX channel of a mode: its offsets (coarse, fine, ultra — 1-based within the mode),
    primary attribute, default and highlight values, and channel functions. */
struct DmxChannel
{
    int                          dmxBreak = 1;
    std::vector<int>             offsets;        // empty => virtual (no DMX footprint)
    juce::String                 geometry;
    juce::String                 attribute;      // from the LogicalChannel
    DmxValue                     defaultValue { 0, 1 };
    bool                         hasHighlight = false;
    DmxValue                     highlight { 0, 1 };
    std::vector<ChannelFunction> functions;

    [[nodiscard]] int  resolution() const noexcept { return juce::jmax (1, static_cast<int> (offsets.size())); }
    [[nodiscard]] bool isVirtual() const noexcept  { return offsets.empty(); }
};

/** A DMX mode: an ordered set of channels; its footprint is the highest offset used. */
struct DmxMode
{
    juce::String            name;
    juce::String            geometry;
    std::vector<DmxChannel> channels;

    [[nodiscard]] int channelCount() const noexcept;
};

/** A parsed GDTF fixture type with its DMX modes. */
struct FixtureType
{
    juce::String         manufacturer;
    juce::String         name;
    juce::String         shortName;
    juce::String         fixtureTypeId;   // GDTF FixtureTypeID (GUID), when present
    juce::String         gdtfVersion;
    std::vector<DmxMode> modes;

    /** Stable identity for patch references: the FixtureTypeID if present, else "Manu / Name". */
    [[nodiscard]] juce::String id() const;
    [[nodiscard]] const DmxMode* findMode (const juce::String& modeName) const;
};

//==============================================================================
// Encoding helpers (used by the engine to turn values into universe bytes).

/** Scales an 8-bit level (0..255) to a channel of @p bytes width, full-range. */
[[nodiscard]] std::uint32_t scaleLevel (std::uint8_t level, int bytes) noexcept;

/** Writes @p value (a @p channel.resolution()-byte integer, coarse-first) into @p out at
    the channel's offsets relative to a 1-based @p startAddress. Out-of-range bytes are
    clipped. Virtual channels write nothing. */
void writeChannelValue (Universe& out, int startAddress, const DmxChannel& channel, std::uint32_t value);
}
