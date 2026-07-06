#include "system_r/core/sacn/SacnReceiver.h"

#include "system_r/core/sacn/E131Packet.h"

namespace systemr
{
SacnReceiver::SacnReceiver (InputMerger& m)
    : juce::Thread ("sACN receiver"), merger (m)
{
}

SacnReceiver::~SacnReceiver()
{
    stop();
}

bool SacnReceiver::start (const std::vector<int>& universes)
{
    bound = socket.bindToPort (e131::kPort);
    if (! bound)
        return false;

    for (int u : universes)
        socket.joinMulticast (e131::multicastAddressFor (u));

    startThread();
    return true;
}

void SacnReceiver::stop()
{
    signalThreadShouldExit();
    socket.shutdown();
    stopThread (2000);
}

bool SacnReceiver::handleDatagram (const std::uint8_t* data, int size, double nowMs)
{
    if (size <= 0)
        return false;

    const auto info = e131::parseDataPacket (data, static_cast<std::size_t> (size));
    if (! info)
        return false;

    if (info->options & e131::kOptionStreamTerminated)
    {
        merger.removeSource (info->universe, info->cid);
        return true;
    }

    merger.ingest (info->universe, info->cid, info->sourceName, info->priority,
                   /*isArtNet*/ false, info->slots, nowMs);
    return true;
}

void SacnReceiver::run()
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
        if (n > 0)
            handleDatagram (buffer.data(), n, juce::Time::getMillisecondCounterHiRes());
    }
}
}
