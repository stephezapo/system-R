#include <catch2/catch_test_macros.hpp>

#include "system_r/core/sacn/E131Packet.h"

using namespace systemr;

TEST_CASE ("multicast address follows E1.31 Appendix A", "[sacn]")
{
    REQUIRE (e131::multicastAddressFor (1)   == "239.255.0.1");
    REQUIRE (e131::multicastAddressFor (256) == "239.255.1.0");
    REQUIRE (e131::multicastAddressFor (513) == "239.255.2.1");
}

TEST_CASE ("data packet matches the E1.31-2018 wire format", "[sacn]")
{
    const juce::Uuid cid ("0123456789abcdef0123456789abcdef");

    Universe slots {};
    slots[0]   = 255;
    slots[511] = 42;

    const auto p = e131::buildDataPacket (cid, "test",
                                          /*priority*/ 100,
                                          /*syncUniverse*/ 0,
                                          /*sequence*/ 7,
                                          /*options*/ 0,
                                          /*universe*/ 1,
                                          slots);

    REQUIRE (p.size() == 638);

    SECTION ("root layer")
    {
        REQUIRE (p[0] == 0x00); REQUIRE (p[1] == 0x10);   // preamble size
        REQUIRE (p[2] == 0x00); REQUIRE (p[3] == 0x00);   // post-amble size

        const std::uint8_t acn[12] =
            { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 };
        for (int i = 0; i < 12; ++i)
            REQUIRE (p[4 + i] == acn[i]);

        REQUIRE (p[16] == 0x72); REQUIRE (p[17] == 0x6e); // flags+len = 622
        REQUIRE (p[18] == 0x00); REQUIRE (p[19] == 0x00);
        REQUIRE (p[20] == 0x00); REQUIRE (p[21] == 0x04); // root vector

        const auto* raw = cid.getRawData();
        for (int i = 0; i < 16; ++i)
            REQUIRE (p[22 + i] == raw[i]);                // CID
    }

    SECTION ("framing layer")
    {
        REQUIRE (p[38] == 0x72); REQUIRE (p[39] == 0x58); // flags+len = 600
        REQUIRE (p[40] == 0x00); REQUIRE (p[41] == 0x00);
        REQUIRE (p[42] == 0x00); REQUIRE (p[43] == 0x02); // framing vector

        REQUIRE (p[44] == 't'); REQUIRE (p[45] == 'e');
        REQUIRE (p[46] == 's'); REQUIRE (p[47] == 't');
        REQUIRE (p[48] == 0x00);                          // null-padded

        REQUIRE (p[108] == 100);                          // priority
        REQUIRE (p[109] == 0x00); REQUIRE (p[110] == 0x00); // sync universe
        REQUIRE (p[111] == 7);                            // sequence
        REQUIRE (p[112] == 0x00);                         // options
        REQUIRE (p[113] == 0x00); REQUIRE (p[114] == 0x01); // universe 1
    }

    SECTION ("DMP layer + slots")
    {
        REQUIRE (p[115] == 0x72); REQUIRE (p[116] == 0x0b); // flags+len = 523
        REQUIRE (p[117] == 0x02);                          // DMP vector
        REQUIRE (p[118] == 0xa1);                          // addr & data type
        REQUIRE (p[119] == 0x00); REQUIRE (p[120] == 0x00); // first prop addr
        REQUIRE (p[121] == 0x00); REQUIRE (p[122] == 0x01); // increment
        REQUIRE (p[123] == 0x02); REQUIRE (p[124] == 0x01); // count = 513
        REQUIRE (p[125] == 0x00);                          // DMX start code
        REQUIRE (p[126] == 255);                           // slot 1
        REQUIRE (p[637] == 42);                            // slot 512
    }
}
