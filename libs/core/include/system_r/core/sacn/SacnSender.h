#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/Universe.h"

#include <cstdint>

namespace systemr
{
/** Sends one universe of DMX over sACN (ANSI E1.31-2018) via IPv4 multicast.

    Phase 1: a single output universe, unsynchronised, default priority. Each source
    gets a persistent random CID and its own per-universe sequence counter.
*/
class SacnSender
{
public:
    SacnSender();

    /** Opens the UDP socket. Returns false if binding failed. */
    bool open();

    void setUniverse (int universe);
    void setSourceName (const juce::String& name);
    void setPriority (std::uint8_t priority);

    /** Packetises and transmits the given slots to the configured universe's
        multicast group. Returns true if the datagram was sent. */
    bool send (const Universe& slots);

    [[nodiscard]] juce::String multicastAddress() const;
    [[nodiscard]] int universe() const noexcept { return outputUniverse; }

private:
    juce::DatagramSocket socket { true }; // enable broadcast/multicast sends
    juce::Uuid cid;
    juce::String sourceName { "system-R" };
    int outputUniverse = 1;
    std::uint8_t priority = 100;
    std::uint8_t sequence = 0;
    bool isOpen = false;
};
}
