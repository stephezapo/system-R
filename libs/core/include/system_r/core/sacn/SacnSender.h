#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/Universe.h"

#include <cstdint>

namespace systemr
{
/** Transmits E1.31 (sACN) data packets from a single shared socket.

    The source identity (CID + source name) is fixed for the console; per-universe
    parameters — universe number, priority, sequence, destination — are passed per
    send so one socket can drive many universes. Sequence numbering per universe is
    the caller's responsibility (see OutputManager). */
class SacnSender
{
public:
    SacnSender();

    /** Opens the UDP socket. Returns false if binding failed. */
    bool open();
    [[nodiscard]] bool isOpen() const noexcept { return opened; }

    void setSourceName (const juce::String& name) { sourceName = name; }
    [[nodiscard]] const juce::Uuid& cid() const noexcept { return cidValue; }

    /** Sends one universe of DMX.
        @param universe     1..63999
        @param priority     0..200 (higher wins on receivers)
        @param sequence     per-universe counter supplied by the caller
        @param syncUniverse sync address, or 0 for unsynchronised
        @param destination  target host; empty selects the universe's multicast group */
    bool sendDmx (std::uint16_t universe,
                  std::uint8_t priority,
                  std::uint8_t sequence,
                  std::uint16_t syncUniverse,
                  const juce::String& destination,
                  const Universe& slots);

    /** IPv4 multicast group for a universe (E1.31 Appendix A). */
    [[nodiscard]] static juce::String multicastAddress (int universe);

private:
    juce::DatagramSocket socket { true }; // enable multicast/broadcast sends
    juce::Uuid cidValue;
    juce::String sourceName { "system-R" };
    bool opened = false;
};
}
