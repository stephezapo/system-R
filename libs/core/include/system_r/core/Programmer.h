#pragma once

#include <atomic>
#include <cstdint>

namespace systemr
{
/** The programmer holds the operator's live edits that override playback.

    Phase 1: a single dimmer level shared by the (one) selected fixture. The value
    is an atomic so the UI thread can set it while the engine thread reads it — the
    lock-free handoff that a structured command FIFO will generalise in later phases.
*/
class Programmer
{
public:
    void setDimmer (std::uint8_t level) noexcept
    {
        dimmer.store (level, std::memory_order_relaxed);
    }

    [[nodiscard]] std::uint8_t dimmerLevel() const noexcept
    {
        return dimmer.load (std::memory_order_relaxed);
    }

private:
    std::atomic<std::uint8_t> dimmer { 0 };
};
}
