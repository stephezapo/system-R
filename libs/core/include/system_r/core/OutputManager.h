#pragma once

#include "system_r/core/OutputConfig.h"
#include "system_r/core/Universe.h"
#include "system_r/core/artnet/ArtNetSender.h"
#include "system_r/core/sacn/SacnSender.h"

#include <cstdint>
#include <vector>

namespace systemr
{
/** Owns the sACN + Art-Net senders and transmits rendered universes according to
    per-universe output config.

    Called from the engine thread once per configured universe per tick. Each universe
    is sent when its data changes and, at minimum, on a keepalive interval so receivers
    don't time the source out. Per-universe sequence numbering lives here so a single
    socket per protocol can drive the full 256-universe budget.

    Config is set once before the engine starts and read (unmutated) thereafter; it is
    not hot-swappable while ticking yet. */
class OutputManager
{
public:
    /** sACN sources must be refreshed well within the 2.5 s source-loss timeout. */
    static constexpr double kSacnKeepaliveMs = 800.0;
    /** Art-Net requires a refresh at least every 4 s; stay comfortably under it. */
    static constexpr double kArtNetKeepaliveMs = 800.0;

    OutputManager();

    /** Opens both underlying sockets. Returns true if at least one opened. */
    bool open();

    void setConfig (std::vector<UniverseOutput> configs);
    [[nodiscard]] const std::vector<UniverseOutput>& config() const noexcept { return universes; }

    void setSourceName (const juce::String& name) { sacn.setSourceName (name); }

    /** Transmits the universe at config position @p configIndex if it changed or a
        keepalive is due. @p nowMs is a monotonic millisecond clock (engine-supplied so
        it is testable). Returns true if a datagram was actually sent. */
    bool send (std::size_t configIndex, const Universe& slots, double nowMs);

    /** Pure transmit decision (change-detection + keepalive), split out so the timing
        logic can be unit-tested without opening sockets. */
    [[nodiscard]] static bool transmitDue (bool valid,
                                           const Universe& lastSent,
                                           double lastSendMs,
                                           const Universe& current,
                                           double nowMs,
                                           double keepaliveMs) noexcept
    {
        if (! valid)
            return true;
        if (current != lastSent)
            return true;
        return (nowMs - lastSendMs) >= keepaliveMs;
    }

private:
    struct Tracked
    {
        Universe lastSent {};
        double   lastSendMs = 0.0;
        bool     valid = false;    // false until the first send
        std::uint8_t sacnSeq = 0;
        std::uint8_t artnetSeq = 0;
    };

    SacnSender sacn;
    ArtNetSender artnet;
    std::vector<UniverseOutput> universes;
    std::vector<Tracked> tracked;
};
}
