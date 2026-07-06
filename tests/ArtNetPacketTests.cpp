#include <catch2/catch_test_macros.hpp>

#include "system_r/core/artnet/ArtNetPacket.h"

using namespace systemr;
using namespace systemr::artnet;

TEST_CASE ("port-address packs Net/Sub-Net/Universe into 15 bits", "[artnet]")
{
    REQUIRE (makePortAddress (0, 0, 0)  == 0x0000);
    REQUIRE (makePortAddress (0, 0, 1)  == 0x0001);
    REQUIRE (makePortAddress (0, 1, 0)  == 0x0010);
    REQUIRE (makePortAddress (1, 0, 0)  == 0x0100);
    REQUIRE (makePortAddress (0x7f, 0xf, 0xf) == 0x7fff);
}

TEST_CASE ("ArtDmx header matches the Art-Net 4 wire format", "[artnet]")
{
    Universe slots {};
    slots[0]   = 255;
    slots[511] = 7;

    const auto p = buildDmxPacket (/*sequence*/ 5, /*physical*/ 0,
                                   makePortAddress (1, 2, 3), slots);

    REQUIRE (p.size() == 530);

    // Identifier "Art-Net\0".
    const std::uint8_t id[8] = { 'A', 'r', 't', '-', 'N', 'e', 't', 0 };
    for (int i = 0; i < 8; ++i)
        REQUIRE (p[static_cast<std::size_t> (i)] == id[i]);

    REQUIRE (p[8] == 0x00); REQUIRE (p[9] == 0x50);  // OpDmx, little-endian
    REQUIRE (p[10] == 0x00); REQUIRE (p[11] == 14);  // ProtVer 14
    REQUIRE (p[12] == 5);                            // Sequence
    REQUIRE (p[13] == 0);                            // Physical
    REQUIRE (p[14] == 0x23);                         // SubUni = sub 2, universe 3
    REQUIRE (p[15] == 0x01);                         // Net = 1
    REQUIRE (p[16] == 0x02); REQUIRE (p[17] == 0x00); // Length 512, big-endian
    REQUIRE (p[18] == 255);                          // slot 1
    REQUIRE (p[529] == 7);                           // slot 512
}

TEST_CASE ("ArtDmx round-trips through parse", "[artnet]")
{
    Universe slots {};
    for (std::size_t i = 0; i < slots.size(); ++i)
        slots[i] = static_cast<std::uint8_t> (i & 0xff);

    const auto port = makePortAddress (3, 5, 9);
    const auto p    = buildDmxPacket (200, 1, port, slots);

    const auto d = parseDmx (p.data(), p.size());
    REQUIRE (d.has_value());
    REQUIRE (d->sequence == 200);
    REQUIRE (d->physical == 1);
    REQUIRE (d->portAddress == port);
    REQUIRE (d->length == 512);
    REQUIRE (d->slots == slots);
}

TEST_CASE ("parseDmx rejects non-Art-Net and wrong op-codes", "[artnet]")
{
    const std::uint8_t junk[20] = { 'n', 'o', 'p', 'e' };
    REQUIRE_FALSE (parseDmx (junk, sizeof (junk)).has_value());

    const auto sync = buildSyncPacket();
    REQUIRE_FALSE (parseDmx (sync.data(), sync.size()).has_value());
    REQUIRE (readOpCode (sync.data(), sync.size()) == OpCode::Sync);
}

TEST_CASE ("ArtSync and ArtPoll carry the right op-codes", "[artnet]")
{
    const auto sync = buildSyncPacket();
    REQUIRE (sync.size() == 14);
    REQUIRE (isArtNet (sync.data(), sync.size()));
    REQUIRE (readOpCode (sync.data(), sync.size()) == OpCode::Sync);
    REQUIRE (sync[11] == 14); // ProtVer

    const auto poll = buildPollPacket();
    REQUIRE (poll.size() == 14);
    REQUIRE (readOpCode (poll.data(), poll.size()) == OpCode::Poll);
}

TEST_CASE ("ArtPollReply round-trips its advertised ports", "[artnet]")
{
    PollReplyConfig cfg;
    cfg.ip = { 10, 0, 0, 42 };
    cfg.shortName = "sysR";
    cfg.longName  = "system-R console";
    cfg.numPorts  = 3;
    cfg.netSwitch = 1;
    cfg.swOut = { 0, 1, 2, 0 };
    cfg.bindIndex = 2;

    const auto p = buildPollReply (cfg);
    REQUIRE (p.size() == 239);
    REQUIRE (readOpCode (p.data(), p.size()) == OpCode::PollReply);

    const auto r = parsePollReply (p.data(), p.size());
    REQUIRE (r.has_value());
    REQUIRE (r->ip == cfg.ip);
    REQUIRE (r->shortName == "sysR");
    REQUIRE (r->longName == "system-R console");
    REQUIRE (r->numPorts == 3);
    REQUIRE (r->netSwitch == 1);
    REQUIRE (r->swOut[1] == 1);
    REQUIRE (r->swOut[2] == 2);
    REQUIRE (r->bindIndex == 2);
}
