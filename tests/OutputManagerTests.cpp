#include <catch2/catch_test_macros.hpp>

#include "system_r/core/OutputConfig.h"
#include "system_r/core/OutputManager.h"
#include "system_r/core/artnet/ArtNetPacket.h"

using namespace systemr;

TEST_CASE ("first frame always transmits", "[output]")
{
    Universe cur {};
    REQUIRE (OutputManager::transmitDue (/*valid*/ false, Universe {}, 0.0, cur, 0.0, 800.0));
}

TEST_CASE ("unchanged data is suppressed until keepalive is due", "[output]")
{
    Universe last {};
    last[0] = 128;
    Universe cur = last; // identical

    // Sent at t=0; nothing changed, well within keepalive -> suppressed.
    REQUIRE_FALSE (OutputManager::transmitDue (true, last, 0.0, cur, 100.0, 800.0));
    REQUIRE_FALSE (OutputManager::transmitDue (true, last, 0.0, cur, 799.0, 800.0));

    // Keepalive elapsed -> retransmit even though data is identical.
    REQUIRE (OutputManager::transmitDue (true, last, 0.0, cur, 800.0, 800.0));
    REQUIRE (OutputManager::transmitDue (true, last, 0.0, cur, 5000.0, 800.0));
}

TEST_CASE ("any change transmits immediately", "[output]")
{
    Universe last {};
    Universe cur {};
    cur[42] = 1; // one slot differs

    REQUIRE (OutputManager::transmitDue (true, last, 0.0, cur, 1.0, 800.0));
}

TEST_CASE ("disabled / none-protocol universes never send", "[output]")
{
    OutputManager mgr;

    UniverseOutput off;
    off.index = 1;
    off.enabled = false;

    UniverseOutput noneProto;
    noneProto.index = 2;
    noneProto.protocol = OutputProtocol::none;

    mgr.setConfig ({ off, noneProto });

    Universe slots {};
    slots[0] = 255;

    // No transmit regardless of sockets, because config gates it out.
    REQUIRE_FALSE (mgr.send (0, slots, 0.0));
    REQUIRE_FALSE (mgr.send (1, slots, 0.0));
    REQUIRE_FALSE (mgr.send (99, slots, 0.0)); // out-of-range index
}

TEST_CASE ("config helpers report the right protocols", "[output]")
{
    UniverseOutput u;

    u.protocol = OutputProtocol::sacn;
    REQUIRE (u.usesSacn());
    REQUIRE_FALSE (u.usesArtNet());

    u.protocol = OutputProtocol::artnet;
    REQUIRE_FALSE (u.usesSacn());
    REQUIRE (u.usesArtNet());

    u.protocol = OutputProtocol::both;
    REQUIRE (u.usesSacn());
    REQUIRE (u.usesArtNet());
}

TEST_CASE ("output config scales to the 256-universe budget", "[output]")
{
    std::vector<UniverseOutput> configs;
    for (int i = 0; i < 256; ++i)
    {
        UniverseOutput cfg;
        cfg.index             = i + 1;
        cfg.sacnUniverse      = static_cast<std::uint16_t> (i + 1);
        cfg.artnetPortAddress = artnet::makePortAddress (i >> 8, (i >> 4) & 0x0f, i & 0x0f);
        configs.push_back (cfg);
    }

    OutputManager mgr;
    mgr.setConfig (configs);
    REQUIRE (mgr.config().size() == 256);
    REQUIRE (mgr.config().back().index == 256);
}
