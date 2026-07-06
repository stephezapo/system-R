#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/gdtf/FixtureType.h"

#include <optional>

namespace systemr::gdtf
{
/** Parses GDTF fixture definitions (ANSI E1.33 / GDTF 1.x).

    A .gdtf file is a ZIP whose `description.xml` holds one `<FixtureType>`. Parsing is
    intentionally defensive: unknown/missing pieces fall back to sensible defaults rather
    than failing, so a broad real-world corpus imports without crashing. The XML entry
    point is exposed for unit testing without touching the filesystem. */

/** Parses a GDTF `description.xml` document (root `<GDTF>` element). */
[[nodiscard]] std::optional<FixtureType> parseDescriptionXml (const juce::String& xml);

/** Parses a GDTF from an already-parsed `<GDTF>` root element. */
[[nodiscard]] std::optional<FixtureType> parseGdtfRoot (const juce::XmlElement& gdtfRoot);

/** Opens a .gdtf archive, extracts `description.xml` and parses it. */
[[nodiscard]] std::optional<FixtureType> parseGdtfFile (const juce::File& gdtfArchive);

/** Parses a .gdtf archive held in memory (e.g. an entry inside an .mvr). */
[[nodiscard]] std::optional<FixtureType> parseGdtfData (const void* data, size_t numBytes);
}
