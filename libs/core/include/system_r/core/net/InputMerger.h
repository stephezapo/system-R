#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/Universe.h"

#include <cstdint>
#include <map>
#include <vector>

namespace systemr
{
/** Tracks inbound DMX sources per universe and resolves them into one merged
    universe, following E1.31 arbitration: highest priority wins; equal-priority
    sources (and Art-Net, which has no priority) are combined HTP (highest-takes-
    precedence) per slot. Sources that fall silent past their protocol timeout are
    dropped from the merge (E1.31 source-loss = 2.5 s).

    The full per-source universe state is retained while a source is live, which is
    the data plumbing for tracking-backup (mirroring another console's output).

    Internally synchronised: receiver threads call ingest()/removeSource() while the
    engine/UI reads merged()/sources(). All timing is caller-supplied (nowMs) so the
    merge is deterministically unit-testable. */
class InputMerger
{
public:
    /** E1.31 source-loss timeout. */
    static constexpr double kSacnTimeoutMs = 2500.0;
    /** Art-Net keepalive is ≤ 4 s; allow a margin before declaring loss. */
    static constexpr double kArtNetTimeoutMs = 5000.0;
    /** Art-Net carries no priority; treat it as the E1.31 default so it HTP-merges
        with default-priority sACN. */
    static constexpr std::uint8_t kArtNetPriority = 100;

    struct SourceInfo
    {
        juce::Uuid   id;
        juce::String name;
        std::uint8_t priority = 0;
        bool         isArtNet = false;
        double       lastSeenMs = 0.0;
    };

    /** Records the latest frame from a source on a universe (adds or refreshes it). */
    void ingest (int universe,
                 const juce::Uuid& sourceId,
                 const juce::String& name,
                 std::uint8_t priority,
                 bool isArtNet,
                 const Universe& slots,
                 double nowMs);

    /** Immediately drops a source (e.g. on an E1.31 stream-terminated packet). */
    void removeSource (int universe, const juce::Uuid& sourceId);

    /** Priority/HTP-merged output for a universe; zeroes if no source is live. */
    [[nodiscard]] Universe merged (int universe, double nowMs) const;

    /** Live sources for a universe (for a source/monitor list). */
    [[nodiscard]] std::vector<SourceInfo> sources (int universe, double nowMs) const;

    /** Tracking-backup: the full captured state of one live source. Returns false if
        that source is absent or has timed out. */
    [[nodiscard]] bool capture (int universe, const juce::Uuid& sourceId,
                                Universe& out, double nowMs) const;

    /** Universe numbers that currently have at least one live source. */
    [[nodiscard]] std::vector<int> liveUniverses (double nowMs) const;

    /** Discards timed-out sources (and now-empty universes) to reclaim memory. */
    void prune (double nowMs);

private:
    struct Source
    {
        juce::Uuid   id;
        juce::String name;
        std::uint8_t priority = 0;
        bool         isArtNet = false;
        double       lastSeenMs = 0.0;
        Universe     slots {};

        [[nodiscard]] bool live (double nowMs) const noexcept
        {
            const double timeout = isArtNet ? kArtNetTimeoutMs : kSacnTimeoutMs;
            return (nowMs - lastSeenMs) < timeout;
        }
    };

    Source* find (std::vector<Source>& list, const juce::Uuid& id) noexcept;

    mutable juce::CriticalSection lock;
    std::map<int, std::vector<Source>> universes;
};
}
