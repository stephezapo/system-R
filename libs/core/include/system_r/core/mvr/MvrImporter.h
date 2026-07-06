#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/ShowDocument.h"
#include "system_r/core/gdtf/FixtureLibrary.h"

#include <optional>
#include <vector>

namespace systemr::mvr
{
/** One fixture parsed from an MVR scene (GeneralSceneDescription.xml). */
struct MvrFixture
{
    juce::String name;
    juce::String uuid;
    juce::String gdtfSpec;    // filename of the GDTF inside the .mvr archive
    juce::String mode;        // GDTFMode name
    juce::String layer;       // owning layer name
    int   fixtureId = 0;
    int   dmxBreak = 1;
    int   universe = 1;       // resolved from the absolute address
    int   address = 1;        // 1..512
    float x = 0.0f, y = 0.0f, z = 0.0f; // position (mm) from the transform matrix
    juce::String gdtfTypeId;  // filled in once the GDTF is imported into the library
};

/** A parsed MVR scene. */
struct MvrScene
{
    std::vector<MvrFixture> fixtures;
    juce::StringArray       layers;
};

/** MVR encodes a single absolute DMX address as (universe-1)*512 + channel. */
[[nodiscard]] int universeFromAbsolute (int absolute) noexcept;
[[nodiscard]] int addressFromAbsolute (int absolute) noexcept;

/** Parses a GeneralSceneDescription.xml document into a scene (no GDTF resolution). */
[[nodiscard]] std::optional<MvrScene> parseSceneXml (const juce::String& xml);

/** Opens a .mvr archive: parses the scene, imports every referenced GDTF into @p library,
    and fills each fixture's gdtfTypeId. Returns the resolved scene. */
[[nodiscard]] std::optional<MvrScene> importMvrFile (const juce::File& mvrArchive,
                                                     gdtf::FixtureLibrary& library);

/** Same as importMvrFile but from an in-memory .mvr archive. */
[[nodiscard]] std::optional<MvrScene> importMvrData (const void* data, size_t numBytes,
                                                     gdtf::FixtureLibrary& library);

/** Adds every resolved fixture in @p scene to @p show, sizing each from its GDTF mode.
    Fixtures are given sequential fids after @p firstFid. Returns the number added. */
int applyToPatch (ShowDocument& show, const gdtf::FixtureLibrary& library,
                  const MvrScene& scene, int firstFid = 1);
}
