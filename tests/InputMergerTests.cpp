#include <catch2/catch_test_macros.hpp>

#include "system_r/core/net/InputMerger.h"

using namespace systemr;

namespace
{
    Universe makeSlots (std::uint8_t v0, std::uint8_t v1 = 0)
    {
        Universe u {};
        u[0] = v0;
        u[1] = v1;
        return u;
    }
}

TEST_CASE ("a single source passes through unchanged", "[merge]")
{
    InputMerger m;
    const juce::Uuid a;
    m.ingest (1, a, "A", 100, false, makeSlots (200, 50), 0.0);

    const auto out = m.merged (1, 0.0);
    REQUIRE (out[0] == 200);
    REQUIRE (out[1] == 50);
}

TEST_CASE ("higher priority wins the whole universe", "[merge]")
{
    InputMerger m;
    const juce::Uuid hi, lo;
    m.ingest (1, hi, "hi", 200, false, makeSlots (10, 0), 0.0);
    m.ingest (1, lo, "lo", 100, false, makeSlots (0, 255), 0.0);

    const auto out = m.merged (1, 0.0);
    REQUIRE (out[0] == 10);   // from the winner
    REQUIRE (out[1] == 0);    // lower-priority source ignored entirely, even where higher
}

TEST_CASE ("equal priority merges HTP per slot", "[merge]")
{
    InputMerger m;
    const juce::Uuid a, b;
    m.ingest (1, a, "A", 100, false, makeSlots (200, 10), 0.0);
    m.ingest (1, b, "B", 100, false, makeSlots (50, 90), 0.0);

    const auto out = m.merged (1, 0.0);
    REQUIRE (out[0] == 200);  // max(200,50)
    REQUIRE (out[1] == 90);   // max(10,90)
}

TEST_CASE ("Art-Net and default-priority sACN HTP-merge together", "[merge]")
{
    InputMerger m;
    const juce::Uuid sacn, art;
    m.ingest (1, sacn, "sacn", InputMerger::kArtNetPriority, false, makeSlots (30, 200), 0.0);
    m.ingest (1, art,  "art",  InputMerger::kArtNetPriority, true,  makeSlots (120, 5), 0.0);

    const auto out = m.merged (1, 0.0);
    REQUIRE (out[0] == 120);
    REQUIRE (out[1] == 200);
}

TEST_CASE ("a source is dropped once its timeout elapses", "[merge]")
{
    InputMerger m;
    const juce::Uuid a;
    m.ingest (1, a, "A", 100, false, makeSlots (200), 0.0);

    REQUIRE (m.merged (1, 1000.0)[0] == 200);                       // still live
    REQUIRE (m.merged (1, InputMerger::kSacnTimeoutMs - 1)[0] == 200);
    REQUIRE (m.merged (1, InputMerger::kSacnTimeoutMs + 1)[0] == 0); // timed out -> released
    REQUIRE (m.sources (1, InputMerger::kSacnTimeoutMs + 1).empty());
}

TEST_CASE ("stream-terminated removal drops a source immediately", "[merge]")
{
    InputMerger m;
    const juce::Uuid a;
    m.ingest (1, a, "A", 100, false, makeSlots (200), 0.0);
    REQUIRE (m.sources (1, 0.0).size() == 1);

    m.removeSource (1, a);
    REQUIRE (m.sources (1, 0.0).empty());
    REQUIRE (m.merged (1, 0.0)[0] == 0);
}

TEST_CASE ("capture exposes the raw per-source state for tracking-backup", "[merge]")
{
    InputMerger m;
    const juce::Uuid a, b;
    m.ingest (1, a, "A", 200, false, makeSlots (10, 20), 0.0);  // wins the merge
    m.ingest (1, b, "B", 100, false, makeSlots (77, 88), 0.0);  // loses, but still captured

    Universe out {};
    REQUIRE (m.capture (1, b, out, 0.0));
    REQUIRE (out[0] == 77);
    REQUIRE (out[1] == 88);

    const juce::Uuid missing;
    REQUIRE_FALSE (m.capture (1, missing, out, 0.0));
}

TEST_CASE ("live universes and prune track source liveness", "[merge]")
{
    InputMerger m;
    const juce::Uuid a, b;
    m.ingest (1, a, "A", 100, false, makeSlots (1), 0.0);
    m.ingest (5, b, "B", 100, false, makeSlots (1), 0.0);

    auto live = m.liveUniverses (0.0);
    REQUIRE (live.size() == 2);

    m.prune (InputMerger::kSacnTimeoutMs + 1);
    REQUIRE (m.liveUniverses (0.0).empty());  // all pruned
}
