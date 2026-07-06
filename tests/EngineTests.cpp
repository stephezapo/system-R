#include <catch2/catch_test_macros.hpp>

#include "GdtfSample.h"

#include "system_r/core/Engine.h"
#include "system_r/core/Programmer.h"
#include "system_r/core/ShowDocument.h"
#include "system_r/core/Universe.h"
#include "system_r/core/gdtf/FixtureLibrary.h"
#include "system_r/core/gdtf/GdtfParser.h"

using namespace systemr;

TEST_CASE ("default show patches one dimmer at universe 1 / address 1", "[engine]")
{
    ShowDocument show;
    const auto fixtures = show.fixtures();

    REQUIRE (fixtures.size() == 1);
    REQUIRE (fixtures[0].universe == 1);
    REQUIRE (fixtures[0].address == 1);
    REQUIRE (fixtures[0].channelCount == 1);
}

TEST_CASE ("render writes the programmer dimmer to the fixture's address", "[engine]")
{
    ShowDocument show;
    gdtf::FixtureLibrary library;
    Programmer programmer;
    Universe out {};

    programmer.setDimmer (200);
    renderUniverse (show, library, programmer, /*universeIndex*/ 1, out);

    REQUIRE (out[0] == 200);   // address 1 -> index 0 (untyped dimmer fallback)
    REQUIRE (out[1] == 0);     // everything else stays dark
}

TEST_CASE ("render only touches the matching universe", "[engine]")
{
    ShowDocument show;
    gdtf::FixtureLibrary library;
    show.addDimmer (2, "Dimmer 2", /*universe*/ 3, /*address*/ 5);

    Programmer programmer;
    programmer.setDimmer (128);

    Universe u1 {};
    renderUniverse (show, library, programmer, 1, u1);
    REQUIRE (u1[0] == 128);    // the default fixture on universe 1
    REQUIRE (u1[4] == 0);

    Universe u3 {};
    renderUniverse (show, library, programmer, 3, u3);
    REQUIRE (u3[4] == 128);    // fixture 2 at address 5 on universe 3
    REQUIRE (u3[0] == 0);
}

TEST_CASE ("render encodes a real GDTF mode (defaults + coarse/fine dimmer)", "[engine][gdtf]")
{
    ShowDocument show;
    show.removeFixture (1); // drop the default untyped dimmer

    gdtf::FixtureLibrary library;
    const auto type = gdtf::parseDescriptionXml (systemr::test::sampleGdtfXml());
    REQUIRE (type.has_value());
    const auto id = library.add (*type);

    // Patch the fixture at universe 1, address 10 (3-channel Standard mode).
    show.addFixture (2, "Spot", id, "Standard", /*universe*/ 1, /*address*/ 10, /*channels*/ 3);

    Programmer programmer;
    Universe out {};

    SECTION ("dimmer full applies the programmer level; Pan keeps its default")
    {
        programmer.setDimmer (255);
        renderUniverse (show, library, programmer, 1, out);

        REQUIRE (out[9]  == 255);   // Dimmer (offset 1 @ addr 10) at full
        REQUIRE (out[10] == 0x80);  // Pan coarse (offset 2 @ addr 11) = default 32768 hi byte
        REQUIRE (out[11] == 0x00);  // Pan fine  (offset 3 @ addr 12)
    }

    SECTION ("dimmer at zero still writes channel defaults")
    {
        programmer.setDimmer (0);
        renderUniverse (show, library, programmer, 1, out);

        REQUIRE (out[9]  == 0);     // Dimmer off
        REQUIRE (out[10] == 0x80);  // Pan default still present (tracking home value)
    }
}
