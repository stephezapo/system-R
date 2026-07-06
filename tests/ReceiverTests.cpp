#include <catch2/catch_test_macros.hpp>

#include "system_r/core/artnet/ArtNetPacket.h"
#include "system_r/core/artnet/ArtNetReceiver.h"
#include "system_r/core/net/InputMerger.h"
#include "system_r/core/sacn/E131Packet.h"
#include "system_r/core/sacn/SacnReceiver.h"

using namespace systemr;

TEST_CASE ("E1.31 data packet round-trips through parse", "[sacn]")
{
    const juce::Uuid cid;
    Universe slots {};
    slots[0]   = 255;
    slots[100] = 64;
    slots[511] = 9;

    const auto packet = e131::buildDataPacket (cid, "desk-A", /*priority*/ 150,
                                               /*syncUniverse*/ 0, /*sequence*/ 12,
                                               /*options*/ 0, /*universe*/ 7, slots);

    const auto info = e131::parseDataPacket (packet.data(), packet.size());
    REQUIRE (info.has_value());
    REQUIRE (info->cid == cid);
    REQUIRE (info->sourceName == "desk-A");
    REQUIRE (info->priority == 150);
    REQUIRE (info->sequence == 12);
    REQUIRE (info->universe == 7);
    REQUIRE (info->slotCount == 512);
    REQUIRE (info->slots == slots);
}

TEST_CASE ("parseDataPacket rejects non-E1.31 input", "[sacn]")
{
    const std::uint8_t junk[200] = { 0 };
    REQUIRE_FALSE (e131::parseDataPacket (junk, sizeof (junk)).has_value());
}

TEST_CASE ("sACN receiver feeds decoded packets into the merger", "[sacn][receiver]")
{
    InputMerger merger;
    SacnReceiver rx (merger);

    const juce::Uuid cid;
    Universe slots {};
    slots[0] = 123;
    const auto packet = e131::buildDataPacket (cid, "src", 100, 0, 0, 0, /*universe*/ 4, slots);

    REQUIRE (rx.handleDatagram (packet.data(), static_cast<int> (packet.size()), 0.0));

    const auto sources = merger.sources (4, 0.0);
    REQUIRE (sources.size() == 1);
    REQUIRE (sources[0].id == cid);
    REQUIRE (merger.merged (4, 0.0)[0] == 123);
}

TEST_CASE ("sACN receiver honours the stream-terminated flag", "[sacn][receiver]")
{
    InputMerger merger;
    SacnReceiver rx (merger);

    const juce::Uuid cid;
    Universe slots {};
    slots[0] = 200;
    const auto data = e131::buildDataPacket (cid, "src", 100, 0, 0, 0, 2, slots);
    REQUIRE (rx.handleDatagram (data.data(), static_cast<int> (data.size()), 0.0));
    REQUIRE (merger.sources (2, 0.0).size() == 1);

    const auto term = e131::buildDataPacket (cid, "src", 100, 0, 1,
                                             e131::kOptionStreamTerminated, 2, slots);
    REQUIRE (rx.handleDatagram (term.data(), static_cast<int> (term.size()), 0.0));
    REQUIRE (merger.sources (2, 0.0).empty());
}

TEST_CASE ("Art-Net receiver ingests ArtDmx keyed by sender IP", "[artnet][receiver]")
{
    InputMerger merger;
    ArtNetReceiver rx (merger);

    Universe slots {};
    slots[0] = 77;
    const auto port = artnet::makePortAddress (0, 0, 3);
    const auto dmx  = artnet::buildDmxPacket (0, 0, port, slots);

    REQUIRE (rx.handleDatagram (dmx.data(), static_cast<int> (dmx.size()), "10.0.0.5", 0.0));

    const auto sources = merger.sources (port, 0.0);
    REQUIRE (sources.size() == 1);
    REQUIRE (sources[0].isArtNet);
    REQUIRE (sources[0].id == ArtNetReceiver::sourceIdForIp ("10.0.0.5"));
    REQUIRE (merger.merged (port, 0.0)[0] == 77);
}

TEST_CASE ("Art-Net source id is stable and IP-derived", "[artnet][receiver]")
{
    REQUIRE (ArtNetReceiver::sourceIdForIp ("2.0.0.1") == ArtNetReceiver::sourceIdForIp ("2.0.0.1"));
    REQUIRE (ArtNetReceiver::sourceIdForIp ("2.0.0.1") != ArtNetReceiver::sourceIdForIp ("2.0.0.2"));
}
