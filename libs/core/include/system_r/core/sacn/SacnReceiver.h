#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/net/InputMerger.h"

#include <vector>

namespace systemr
{
/** Receives E1.31 (sACN) data packets and feeds them into an InputMerger.

    Runs its own thread listening on UDP 5568, joined to the multicast group of each
    universe of interest. Packet decoding is exposed via handleDatagram() so the merge
    plumbing is unit-testable without a live socket. */
class SacnReceiver : private juce::Thread
{
public:
    explicit SacnReceiver (InputMerger& merger);
    ~SacnReceiver() override;

    /** Binds the socket, joins the given universes' multicast groups and starts the
        listen thread. Returns false if the socket could not be bound. */
    bool start (const std::vector<int>& universes);
    void stop();

    /** Decodes one received datagram and updates the merger. Returns true if it was a
        valid E1.31 data packet (ingested, or a stream-terminated release). */
    bool handleDatagram (const std::uint8_t* data, int size, double nowMs);

private:
    void run() override;

    InputMerger& merger;
    juce::DatagramSocket socket { true };
    bool bound = false;
};
}
