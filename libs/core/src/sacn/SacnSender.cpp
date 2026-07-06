#include "system_r/core/sacn/SacnSender.h"

#include "system_r/core/sacn/E131Packet.h"

namespace systemr
{
SacnSender::SacnSender() = default;

bool SacnSender::open()
{
    // Bind to an ephemeral local port; multicast sends don't require joining a group.
    opened = socket.bindToPort (0);
    return opened;
}

juce::String SacnSender::multicastAddress (int universe)
{
    return e131::multicastAddressFor (universe);
}

bool SacnSender::sendDmx (std::uint16_t universe,
                          std::uint8_t priority,
                          std::uint8_t sequence,
                          std::uint16_t syncUniverse,
                          const juce::String& destination,
                          const Universe& slots)
{
    if (! opened)
        return false;

    const auto packet = e131::buildDataPacket (cidValue, sourceName, priority,
                                               syncUniverse, sequence, /*options*/ 0,
                                               universe, slots);

    const auto host = destination.isNotEmpty() ? destination
                                               : multicastAddress (universe);
    const int sent = socket.write (host, e131::kPort,
                                   packet.data(), static_cast<int> (packet.size()));
    return sent == static_cast<int> (packet.size());
}
}
