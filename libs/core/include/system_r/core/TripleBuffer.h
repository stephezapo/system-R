#pragma once

#include <atomic>
#include <cstdint>

namespace systemr
{
/** A lock-free single-producer / single-consumer triple buffer.

    The producer renders into writeSlot() and calls publish(); the consumer calls
    acquire() (true if a newer value was picked up) and reads readSlot(). Writer,
    middle and reader always occupy three distinct slots, so the producer never
    blocks and the consumer always sees a complete, untorn value.

    Note: under a publish that races an acquire, the consumer may clear a freshly
    set flag and observe the new value one cycle later. It never returns torn data
    and never aliases the producer's slot — acceptable for engine→UI snapshotting.
*/
template <typename T>
class TripleBuffer
{
public:
    TripleBuffer() { middle.store (makeState (2, false), std::memory_order_relaxed); }

    // --- producer side ---
    [[nodiscard]] T& writeSlot() noexcept { return slots[writeIndex]; }

    void publish() noexcept
    {
        const State prev = middle.exchange (makeState (writeIndex, true),
                                            std::memory_order_acq_rel);
        writeIndex = indexOf (prev);
    }

    // --- consumer side ---
    /** Returns true and swaps in the latest value if one was published since the
        last acquire; otherwise leaves readSlot() unchanged and returns false. */
    [[nodiscard]] bool acquire() noexcept
    {
        if (! freshOf (middle.load (std::memory_order_acquire)))
            return false;

        const State prev = middle.exchange (makeState (readIndex, false),
                                            std::memory_order_acq_rel);
        readIndex = indexOf (prev);
        return true;
    }

    [[nodiscard]] const T& readSlot() const noexcept { return slots[readIndex]; }

private:
    using State = std::uint32_t;
    static constexpr State makeState (int idx, bool fresh) noexcept
    {
        return static_cast<State> ((idx & 0x3) | (fresh ? 0x4 : 0x0));
    }
    static constexpr int  indexOf (State s) noexcept { return static_cast<int> (s & 0x3); }
    static constexpr bool freshOf (State s) noexcept { return (s & 0x4) != 0; }

    T slots[3] {};
    int writeIndex = 0;                 // producer-owned
    int readIndex  = 1;                 // consumer-owned
    std::atomic<State> middle;          // shared: index of the published slot + fresh flag
};
}
