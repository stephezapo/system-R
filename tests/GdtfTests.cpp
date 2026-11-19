#include <catch2/catch_test_macros.hpp>

#include "GdtfSample.h"

#include "system_r/core/gdtf/FixtureLibrary.h"
#include "system_r/core/gdtf/FixtureType.h"
#include "system_r/core/gdtf/GdtfParser.h"

using namespace systemr;
using namespace systemr::gdtf;

TEST_CASE ("DMXValue parses value/bytes notation", "[gdtf]")
{
    REQUIRE (DmxValue::parse ("0/1").value == 0);
    REQUIRE (DmxValue::parse ("255/1").bytes == 1);
    REQUIRE (DmxValue::parse ("32768/2").value == 32768);
    REQUIRE (DmxValue::parse ("32768/2").bytes == 2);
    REQUIRE (DmxValue::parse ("77").value == 77);       // width omitted -> default
    REQUIRE (DmxValue::parse ("").value == 0);
}

TEST_CASE ("DMXValue rescales between channel widths (GDTF bit rule)", "[gdtf]")
{
    REQUIRE (DmxValue { 255, 1 }.toChannelValue (2) == 65535); // widen: replicate top byte
    REQUIRE (DmxValue { 0, 1 }.toChannelValue (2) == 0);
    REQUIRE (DmxValue { 65535, 2 }.toChannelValue (1) == 255); // narrow: keep MSB
    REQUIRE (DmxValue { 32768, 2 }.toChannelValue (2) == 32768);
}

TEST_CASE ("scaleLevel maps 8-bit levels full-range", "[gdtf]")
{
    REQUIRE (scaleLevel (0, 1) == 0);
    REQUIRE (scaleLevel (255, 1) == 255);
    REQUIRE (scaleLevel (128, 1) == 128);
    REQUIRE (scaleLevel (0, 2) == 0);
    REQUIRE (scaleLevel (255, 2) == 65535); // full 16-bit at full level
}

TEST_CASE ("writeChannelValue splits coarse/fine at the right addresses", "[gdtf]")
{
    Universe out {};
    DmxChannel pan;
    pan.offsets = { 2, 3 };               // 16-bit at mode-offset 2 (coarse) / 3 (fine)

    writeChannelValue (out, /*startAddress*/ 10, pan, 0x8001);
    REQUIRE (out[10] == 0x80);  // address 11 (10 + 2 - 1) = coarse
    REQUIRE (out[11] == 0x01);  // address 12 (10 + 3 - 1) = fine
    REQUIRE (out[9] == 0);      // dimmer slot untouched
}

TEST_CASE ("GDTF description parses type, modes and channels", "[gdtf]")
{
    const auto type = parseDescriptionXml (test::sampleGdtfXml());
    REQUIRE (type.has_value());

    REQUIRE (type->manufacturer == "Acme");
    REQUIRE (type->name == "Test Spot");
    REQUIRE (type->shortName == "TSpot");
    REQUIRE (type->id() == "11111111-2222-3333-4444-555555555555");
    REQUIRE (type->modes.size() == 1);

    const auto* mode = type->findMode ("Standard");
    REQUIRE (mode != nullptr);
    REQUIRE (mode->channelCount() == 3);        // max offset across channels
    REQUIRE (mode->channels.size() == 3);

    const auto& dimmer = mode->channels[0];
    REQUIRE (dimmer.attribute == "Dimmer");
    REQUIRE (dimmer.offsets == std::vector<int> { 1 });
    REQUIRE (dimmer.resolution() == 1);
    REQUIRE (dimmer.hasHighlight);
    REQUIRE (dimmer.highlight.value == 255);
    REQUIRE (dimmer.functions.size() == 1);
    REQUIRE (dimmer.functions[0].physicalTo == 100.0);

    const auto& pan = mode->channels[1];
    REQUIRE (pan.attribute == "Pan");
    REQUIRE (pan.offsets == std::vector<int> { 2, 3 });
    REQUIRE (pan.resolution() == 2);
    REQUIRE (pan.defaultValue.value == 32768);

    const auto& macro = mode->channels[2];
    REQUIRE (macro.isVirtual());
    REQUIRE (macro.offsets.empty());
}

TEST_CASE ("GDTF parser is defensive against junk", "[gdtf]")
{
    REQUIRE_FALSE (parseDescriptionXml ("not xml at all").has_value());
    REQUIRE_FALSE (parseDescriptionXml ("<GDTF></GDTF>").has_value()); // no FixtureType
}

TEST_CASE ("GDTF parses from a real .gdtf zip archive", "[gdtf]")
{
    // Build a minimal .gdtf (a ZIP containing description.xml) in memory.
    const auto xml = test::sampleGdtfXml();
    juce::ZipFile::Builder builder;
    builder.addEntry (std::make_unique<juce::MemoryInputStream> (xml.toRawUTF8(), xml.getNumBytesAsUTF8(), false),
                      6, "description.xml", juce::Time::getCurrentTime());

    juce::MemoryOutputStream zipped;
    REQUIRE (builder.writeToStream (zipped, nullptr));

    const auto type = parseGdtfData (zipped.getData(), zipped.getDataSize());
    REQUIRE (type.has_value());
    REQUIRE (type->name == "Test Spot");
    REQUIRE (type->findMode ("Standard") != nullptr);
}

TEST_CASE ("FixtureLibrary stores, replaces and looks up types", "[gdtf][library]")
{
    FixtureLibrary lib;
    const auto type = parseDescriptionXml (test::sampleGdtfXml());
    REQUIRE (type.has_value());

    const auto id = lib.add (*type);
    REQUIRE (id == "11111111-2222-3333-4444-555555555555");
    REQUIRE (lib.size() == 1);
    REQUIRE (lib.find (id) != nullptr);
    REQUIRE (lib.findMode (id, "Standard") != nullptr);
    REQUIRE (lib.findMode (id, "Nonexistent") == nullptr);

    const auto grouped = lib.byManufacturer();
    REQUIRE (grouped.count ("Acme") == 1);

    lib.add (*type);              // same id -> replaced, not duplicated
    REQUIRE (lib.size() == 1);
}
