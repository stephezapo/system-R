#include "system_r/core/OutputManager.h"

namespace systemr
{
OutputManager::OutputManager() = default;

bool OutputManager::open()
{
    const bool a = sacn.open();
    const bool b = artnet.open();
    return a || b;
}

void OutputManager::setConfig (std::vector<UniverseOutput> configs)
{
    universes = std::move (configs);
    tracked.assign (universes.size(), Tracked {});
}

bool OutputManager::send (std::size_t configIndex, const Universe& slots, double nowMs)
{
    if (configIndex >= universes.size())
        return false;

    const auto& cfg = universes[configIndex];
    auto& t = tracked[configIndex];

    if (! cfg.enabled || cfg.protocol == OutputProtocol::none)
        return false;

    const double keepalive = juce::jmin (kSacnKeepaliveMs, kArtNetKeepaliveMs);
    if (! transmitDue (t.valid, t.lastSent, t.lastSendMs, slots, nowMs, keepalive))
        return false;

    bool sent = false;

    if (cfg.usesSacn())
    {
        if (sacn.sendDmx (cfg.sacnUniverse, cfg.priority, t.sacnSeq,
                          /*syncUniverse*/ 0, cfg.sacnDestination, slots))
        {
            ++t.sacnSeq;
            sent = true;
        }
    }

    if (cfg.usesArtNet())
    {
        if (artnet.sendDmx (cfg.artnetPortAddress, t.artnetSeq,
                            cfg.artnetDestination, slots))
        {
            if (++t.artnetSeq == 0) // Art-Net sequence 0 means "disabled"; skip it
                t.artnetSeq = 1;
            sent = true;
        }
    }

    t.lastSent = slots;
    t.lastSendMs = nowMs;
    t.valid = true;
    return sent;
}
}
