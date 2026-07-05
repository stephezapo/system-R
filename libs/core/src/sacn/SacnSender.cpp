#include "system_r/core/sacn/SacnSender.h"

#include "system_r/core/sacn/E131Packet.h"

namespace systemr
{
SacnSender::SacnSender() = default;

bool SacnSender::open()
{
    // Bind to an ephemeral local port; multicast sends don't require joining a group.
    isOpen = socket.bindToPort (0);
    return isOpen;
}

void SacnSender::setUniverse (int universe)      { outputUniverse = universe; }
void SacnSender::setSourceName (const juce::String& name) { sourceName = name; }
void SacnSender::setPriority (std::uint8_t p)    { priority = p; }

juce::String SacnSender::multicastAddress() const
{
    return e131::multicastAddressFor (outputUniverse);
}

bool SacnSender::send (const Universe& slots)
{
    if (! isOpen)
        return false;

    const auto packet = e131::buildDataPacket (cid, sourceName, priority,
                                               /*syncUniverse*/ 0, sequence,
                                               /*options*/ 0,
                                               static_cast<std::uint16_t> (outputUniverse),
                                               slots);
    ++sequence; // wraps naturally at 256

    const int sent = socket.write (multicastAddress(), e131::kPort,
                                   packet.data(), static_cast<int> (packet.size()));
    return sent == static_cast<int> (packet.size());
}
}
