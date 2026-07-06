#pragma once

#include <juce_core/juce_core.h>

#include <cstdint>

namespace systemr
{
/** Which wire protocol(s) a logical universe is transmitted on. */
enum class OutputProtocol
{
    none,
    sacn,
    artnet,
    both,
};

/** Output configuration for one logical universe.

    The logical @c index is the engine's buffer slot (1..N); it is decoupled from the
    on-wire addressing so the same rendered universe can be sent to any sACN universe
    number and/or Art-Net Port-Address, to unicast/broadcast/multicast destinations. */
struct UniverseOutput
{
    int index = 1;                                   // logical universe / engine buffer
    bool enabled = true;
    OutputProtocol protocol = OutputProtocol::sacn;

    // --- sACN (E1.31) ---
    std::uint16_t sacnUniverse = 1;                  // 1..63999
    std::uint8_t  priority = 100;                    // 0..200
    juce::String  sacnDestination;                   // empty => Appendix-A multicast group

    // --- Art-Net ---
    std::uint16_t artnetPortAddress = 0;             // 15-bit Net/Sub-Net/Universe
    juce::String  artnetDestination { "255.255.255.255" }; // broadcast or a node's unicast IP

    [[nodiscard]] bool usesSacn() const noexcept
    {
        return protocol == OutputProtocol::sacn || protocol == OutputProtocol::both;
    }

    [[nodiscard]] bool usesArtNet() const noexcept
    {
        return protocol == OutputProtocol::artnet || protocol == OutputProtocol::both;
    }
};
}
