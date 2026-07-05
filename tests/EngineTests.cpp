#include <catch2/catch_test_macros.hpp>

#include "system_r/core/Engine.h"
#include "system_r/core/Programmer.h"
#include "system_r/core/ShowDocument.h"
#include "system_r/core/Universe.h"

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
    Programmer programmer;
    Universe out {};

    programmer.setDimmer (200);
    renderUniverse (show, programmer, /*universeIndex*/ 1, out);

    REQUIRE (out[0] == 200);   // address 1 -> index 0
    REQUIRE (out[1] == 0);     // everything else stays dark
}

TEST_CASE ("render only touches the matching universe", "[engine]")
{
    ShowDocument show;
    show.addDimmer (2, "Dimmer 2", /*universe*/ 3, /*address*/ 5);

    Programmer programmer;
    programmer.setDimmer (128);

    Universe u1 {};
    renderUniverse (show, programmer, 1, u1);
    REQUIRE (u1[0] == 128);    // the default fixture on universe 1
    REQUIRE (u1[4] == 0);

    Universe u3 {};
    renderUniverse (show, programmer, 3, u3);
    REQUIRE (u3[4] == 128);    // fixture 2 at address 5 on universe 3
    REQUIRE (u3[0] == 0);
}
