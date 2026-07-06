#include <catch2/catch_test_macros.hpp>

#include "GdtfSample.h"

#include "system_r/core/ShowDocument.h"
#include "system_r/core/gdtf/FixtureLibrary.h"
#include "system_r/core/mvr/MvrImporter.h"

using namespace systemr;

namespace
{
    juce::String sampleSceneXml()
    {
        return R"(<?xml version="1.0" encoding="UTF-8"?>
<GeneralSceneDescription verMajor="1" verMinor="5">
  <Scene>
    <Layers>
      <Layer name="Layer 1" uuid="L1">
        <ChildList>
          <Fixture name="Spot 1" uuid="F1">
            <GDTFSpec>Acme@Test Spot.gdtf</GDTFSpec>
            <GDTFMode>Standard</GDTFMode>
            <FixtureID>1</FixtureID>
            <Addresses><Address break="1">10</Address></Addresses>
            <Matrix>{1,0,0}{0,1,0}{0,0,1}{1000,2000,3000}</Matrix>
          </Fixture>
          <GroupObject name="Group" uuid="G1">
            <ChildList>
              <Fixture name="Spot 2" uuid="F2">
                <GDTFSpec>Acme@Test Spot.gdtf</GDTFSpec>
                <GDTFMode>Standard</GDTFMode>
                <FixtureID>2</FixtureID>
                <Addresses><Address break="1">513</Address></Addresses>
                <Matrix>{1,0,0}{0,1,0}{0,0,1}{0,0,0}</Matrix>
              </Fixture>
            </ChildList>
          </GroupObject>
        </ChildList>
      </Layer>
    </Layers>
  </Scene>
</GeneralSceneDescription>)";
    }

    // Wraps the sample GDTF into an in-memory .gdtf (ZIP) and returns its bytes.
    juce::MemoryBlock buildGdtfArchive()
    {
        const auto xml = systemr::test::sampleGdtfXml();
        juce::ZipFile::Builder b;
        b.addEntry (new juce::MemoryInputStream (xml.toRawUTF8(), xml.getNumBytesAsUTF8(), false),
                    0, "description.xml", juce::Time::getCurrentTime());
        juce::MemoryOutputStream out;
        b.writeToStream (out, nullptr);
        return out.getMemoryBlock();
    }
}

TEST_CASE ("MVR absolute address maps to universe + channel", "[mvr]")
{
    REQUIRE (mvr::universeFromAbsolute (1) == 1);
    REQUIRE (mvr::addressFromAbsolute (1) == 1);
    REQUIRE (mvr::universeFromAbsolute (512) == 1);
    REQUIRE (mvr::addressFromAbsolute (512) == 512);
    REQUIRE (mvr::universeFromAbsolute (513) == 2);
    REQUIRE (mvr::addressFromAbsolute (513) == 1);
    REQUIRE (mvr::universeFromAbsolute (1025) == 3);
}

TEST_CASE ("MVR scene parses layers, nested fixtures, address and position", "[mvr]")
{
    const auto scene = mvr::parseSceneXml (sampleSceneXml());
    REQUIRE (scene.has_value());

    REQUIRE (scene->layers.size() == 1);
    REQUIRE (scene->layers[0] == "Layer 1");
    REQUIRE (scene->fixtures.size() == 2); // one top-level, one inside a GroupObject

    const auto& a = scene->fixtures[0];
    REQUIRE (a.name == "Spot 1");
    REQUIRE (a.gdtfSpec == "Acme@Test Spot.gdtf");
    REQUIRE (a.mode == "Standard");
    REQUIRE (a.fixtureId == 1);
    REQUIRE (a.universe == 1);
    REQUIRE (a.address == 10);
    REQUIRE (a.x == 1000.0f);
    REQUIRE (a.y == 2000.0f);
    REQUIRE (a.z == 3000.0f);

    const auto& b = scene->fixtures[1];
    REQUIRE (b.name == "Spot 2");
    REQUIRE (b.layer == "Layer 1");
    REQUIRE (b.universe == 2); // absolute 513 -> universe 2, address 1
    REQUIRE (b.address == 1);
}

TEST_CASE ("MVR import resolves embedded GDTFs and patches the show", "[mvr][gdtf]")
{
    // Build an in-memory .mvr: GeneralSceneDescription.xml + the referenced .gdtf.
    const auto scene = sampleSceneXml();
    const auto gdtfArchive = buildGdtfArchive();

    juce::ZipFile::Builder b;
    b.addEntry (new juce::MemoryInputStream (scene.toRawUTF8(), scene.getNumBytesAsUTF8(), false),
                0, "GeneralSceneDescription.xml", juce::Time::getCurrentTime());
    b.addEntry (new juce::MemoryInputStream (gdtfArchive.getData(), gdtfArchive.getSize(), false),
                0, "Acme@Test Spot.gdtf", juce::Time::getCurrentTime());

    juce::MemoryOutputStream mvr;
    REQUIRE (b.writeToStream (mvr, nullptr));

    gdtf::FixtureLibrary library;
    const auto imported = mvr::importMvrData (mvr.getData(), mvr.getDataSize(), library);
    REQUIRE (imported.has_value());
    REQUIRE (library.size() == 1); // the one referenced type, imported once

    REQUIRE (imported->fixtures.size() == 2);
    for (const auto& f : imported->fixtures)
        REQUIRE (f.gdtfTypeId == "11111111-2222-3333-4444-555555555555");

    ShowDocument show;
    const int added = mvr::applyToPatch (show, library, *imported, /*firstFid*/ 100);
    REQUIRE (added == 2);

    const auto fixtures = show.fixtures();
    const auto it = std::find_if (fixtures.begin(), fixtures.end(),
                                  [] (const auto& f) { return f.fid == 100; });
    REQUIRE (it != fixtures.end());
    REQUIRE (it->channelCount == 3);  // sized from the GDTF Standard mode
    REQUIRE (it->universe == 1);
    REQUIRE (it->address == 10);
}
