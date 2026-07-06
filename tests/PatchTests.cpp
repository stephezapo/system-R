#include <catch2/catch_test_macros.hpp>

#include "system_r/core/Patch.h"
#include "system_r/core/ShowDocument.h"

using namespace systemr;

namespace
{
    ShowDocument::Fixture fx (int fid, int universe, int address, int channels)
    {
        ShowDocument::Fixture f;
        f.fid = fid;
        f.universe = universe;
        f.address = address;
        f.channelCount = channels;
        return f;
    }
}

TEST_CASE ("rangesOverlap detects channel-range intersection", "[patch]")
{
    REQUIRE (patch::rangesOverlap (1, 3, 3, 1));    // 1..3 vs 3
    REQUIRE_FALSE (patch::rangesOverlap (1, 3, 4, 1)); // 1..3 vs 4
    REQUIRE (patch::rangesOverlap (5, 2, 4, 4));    // fully contained
}

TEST_CASE ("wouldOverlap checks a candidate against the patch", "[patch]")
{
    const std::vector<ShowDocument::Fixture> existing { fx (1, 1, 1, 3) }; // U1 addr 1..3

    REQUIRE (patch::wouldOverlap (existing, 1, 3, 1));      // hits address 3
    REQUIRE_FALSE (patch::wouldOverlap (existing, 1, 4, 1)); // just clear
    REQUIRE_FALSE (patch::wouldOverlap (existing, 2, 1, 3)); // different universe
    REQUIRE_FALSE (patch::wouldOverlap (existing, 1, 1, 3, /*ignoreFid*/ 1)); // ignores itself
}

TEST_CASE ("overlappingFixtures reports all conflicting fids", "[patch]")
{
    const std::vector<ShowDocument::Fixture> fixtures {
        fx (1, 1, 1, 4),   // 1..4
        fx (2, 1, 4, 2),   // 4..5  -> overlaps fixture 1
        fx (3, 1, 10, 3),  // 10..12 -> clear
    };

    const auto conflicts = patch::overlappingFixtures (fixtures);
    REQUIRE (conflicts.count (1) == 1);
    REQUIRE (conflicts.count (2) == 1);
    REQUIRE (conflicts.count (3) == 0);
}

TEST_CASE ("firstFreeAddress finds the next non-overlapping slot", "[patch]")
{
    const std::vector<ShowDocument::Fixture> fixtures { fx (1, 1, 1, 10) }; // 1..10

    REQUIRE (patch::firstFreeAddress (fixtures, 1, 5) == 11);
    REQUIRE (patch::firstFreeAddress (fixtures, 2, 5) == 1);   // empty universe
    REQUIRE (patch::firstFreeAddress (fixtures, 1, 513) == -1); // won't fit a universe
}

TEST_CASE ("ShowDocument stores and removes GDTF-typed fixtures", "[patch]")
{
    ShowDocument show;
    show.addFixture (7, "Spot 1", "type-id", "Standard", /*universe*/ 2, /*address*/ 20, /*channels*/ 16);

    auto fixtures = show.fixtures();
    const auto it = std::find_if (fixtures.begin(), fixtures.end(),
                                  [] (const auto& f) { return f.fid == 7; });
    REQUIRE (it != fixtures.end());
    REQUIRE (it->gdtfTypeId == "type-id");
    REQUIRE (it->modeName == "Standard");
    REQUIRE (it->channelCount == 16);
    REQUIRE (it->universe == 2);
    REQUIRE (it->address == 20);

    REQUIRE (show.removeFixture (7));
    REQUIRE_FALSE (show.removeFixture (7)); // already gone
}
