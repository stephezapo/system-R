#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/artnet/ArtNetPacket.h"
#include "system_r/core/net/InputMerger.h"

#include <atomic>

namespace systemr
{
/** Receives ArtDmx into an InputMerger and answers ArtPoll with an ArtPollReply.

    Art-Net has no source CID, so a source is identified by its sender IP and merged at
    the E1.31 default priority (HTP with equal-priority sACN). Packet decoding is exposed
    via handleDatagram() so the plumbing is unit-testable without a live socket. */
class ArtNetReceiver : private juce::Thread
{
public:
    explicit ArtNetReceiver (InputMerger& merger);
    ~ArtNetReceiver() override;

    /** Advertises these ports in ArtPollReply responses (optional; set before start). */
    void setPollReply (const artnet::PollReplyConfig& cfg);

    /** Binds UDP 6454 and starts the listen thread. Returns false if binding failed. */
    bool start();
    void stop();

    /** Derives a stable per-node source id from a dotted-quad sender address. */
    [[nodiscard]] static juce::Uuid sourceIdForIp (const juce::String& ip);

    /** Decodes one datagram: ingests ArtDmx into the merger. Returns true if it was an
        ArtDmx packet. (ArtPoll replies are driven by the socket loop, not here.) */
    bool handleDatagram (const std::uint8_t* data, int size,
                         const juce::String& senderIp, double nowMs);

private:
    void run() override;

    InputMerger& merger;
    juce::DatagramSocket socket { true };
    artnet::PollReplyConfig pollReply;
    std::atomic<bool> hasPollReply { false };
    bool bound = false;
};
}
