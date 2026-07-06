#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/Universe.h"

#include <cstdint>

namespace systemr
{
/** Transmits ArtDmx / ArtSync over UDP from a single shared socket.

    Art-Net has no per-packet priority and no multicast; frames are sent to a
    destination host (limited/subnet broadcast or a node's unicast address) on
    port 6454. Per Port-Address sequence numbering is the caller's responsibility
    so one socket can drive many universes — see OutputManager. */
class ArtNetSender
{
public:
    ArtNetSender();

    /** Opens the UDP socket with broadcast enabled. Returns false if binding failed. */
    bool open();
    [[nodiscard]] bool isOpen() const noexcept { return opened; }

    /** Sends one full-universe ArtDmx frame to @p destination.
        @param portAddress 15-bit Art-Net Port-Address (Net/Sub-Net/Universe)
        @param sequence    0 disables ordering; otherwise the caller's per-address counter
        @param destination dotted-quad host (broadcast or unicast) */
    bool sendDmx (std::uint16_t portAddress,
                  std::uint8_t sequence,
                  const juce::String& destination,
                  const Universe& slots);

    /** Sends an ArtSync to release buffered frames at @p destination. */
    bool sendSync (const juce::String& destination);

private:
    juce::DatagramSocket socket { true }; // broadcast-enabled
    bool opened = false;
};
}
