#include "system_r/core/artnet/ArtNetSender.h"

#include "system_r/core/artnet/ArtNetPacket.h"

namespace systemr
{
ArtNetSender::ArtNetSender() = default;

bool ArtNetSender::open()
{
    opened = socket.bindToPort (0); // ephemeral local port; sends only
    return opened;
}

bool ArtNetSender::sendDmx (std::uint16_t portAddress,
                            std::uint8_t sequence,
                            const juce::String& destination,
                            const Universe& slots)
{
    if (! opened)
        return false;

    const auto packet = artnet::buildDmxPacket (sequence, /*physical*/ 0, portAddress, slots);
    const int sent = socket.write (destination, artnet::kPort,
                                   packet.data(), static_cast<int> (packet.size()));
    return sent == static_cast<int> (packet.size());
}

bool ArtNetSender::sendSync (const juce::String& destination)
{
    if (! opened)
        return false;

    const auto packet = artnet::buildSyncPacket();
    const int sent = socket.write (destination, artnet::kPort,
                                   packet.data(), static_cast<int> (packet.size()));
    return sent == static_cast<int> (packet.size());
}
}
