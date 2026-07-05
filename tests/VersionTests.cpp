#include <catch2/catch_test_macros.hpp>

#include "system_r/core/Version.h"

TEST_CASE ("core reports a non-empty version", "[core]")
{
    REQUIRE_FALSE (systemr::coreVersion().empty());
}

TEST_CASE ("core version matches the expected value", "[core]")
{
    REQUIRE (systemr::coreVersion() == "0.0.1");
}
