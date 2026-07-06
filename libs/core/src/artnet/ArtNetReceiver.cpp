#include "system_r/core/artnet/ArtNetReceiver.h"

#include <array>

namespace systemr
{
ArtNetReceiver::ArtNetReceiver (InputMerger& m)
    : juce::Thread ("Art-Net receiver"), merger (m)
{
}

ArtNetReceiver::~ArtNetReceiver()
{
    stop();
}

void ArtNetReceiver::setPollReply (const artnet::PollReplyConfig& cfg)
{
    pollReply = cfg;
    hasPollReply = true;
}

bool ArtNetReceiver::start()
{
    bound = socket.bindToPort (artnet::kPort);
    if (! bound)
        return false;

    startThread();
    return true;
}

void ArtNetReceiver::stop()
{
    signalThreadShouldExit();
    socket.shutdown();
    stopThread (2000);
}

juce::Uuid ArtNetReceiver::sourceIdForIp (const juce::String& ip)
{
    // Deterministic 16-byte id from the four IP octets; a node == its address.
    std::array<std::uint8_t, 16> bytes {};
    auto parts = juce::StringArray::fromTokens (ip, ".", "");
    for (int i = 0; i < juce::jmin (4, parts.size()); ++i)
        bytes[static_cast<std::size_t> (i)] = static_cast<std::uint8_t> (parts[i].getIntValue() & 0xff);
    return juce::Uuid (bytes.data());
}

bool ArtNetReceiver::handleDatagram (const std::uint8_t* data, int size,
                                     const juce::String& senderIp, double nowMs)
{
    if (size <= 0)
        return false;

    const auto dmx = artnet::parseDmx (data, static_cast<std::size_t> (size));
    if (! dmx)
        return false;

    merger.ingest (dmx->portAddress, sourceIdForIp (senderIp), senderIp,
                   InputMerger::kArtNetPriority, /*isArtNet*/ true, dmx->slots, nowMs);
    return true;
}

void ArtNetReceiver::run()
{
    std::vector<std::uint8_t> buffer (1500);

    while (! threadShouldExit())
    {
        const int ready = socket.waitUntilReady (true, 200);
        if (ready < 0)
            break;
        if (ready == 0)
            continue;

        juce::String senderIp;
        int senderPort = 0;
        const int n = socket.read (buffer.data(), static_cast<int> (buffer.size()),
                                   false, senderIp, senderPort);
        if (n <= 0)
            continue;

        const double nowMs = juce::Time::getMillisecondCounterHiRes();

        if (handleDatagram (buffer.data(), n, senderIp, nowMs))
            continue;

        // Answer discovery polls with our advertised ports.
        if (hasPollReply.load()
            && artnet::readOpCode (buffer.data(), static_cast<std::size_t> (n)) == artnet::OpCode::Poll)
        {
            const auto reply = artnet::buildPollReply (pollReply);
            socket.write (senderIp, artnet::kPort, reply.data(), static_cast<int> (reply.size()));
        }
    }
}
}
