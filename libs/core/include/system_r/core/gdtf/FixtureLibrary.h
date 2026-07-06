#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/gdtf/FixtureType.h"

#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace systemr::gdtf
{
/** Holds imported GDTF fixture types, keyed by their stable id().

    Stored via unique_ptr so pointers handed out by find() stay valid as more types are
    imported — the patch and engine cache DmxMode pointers into these entries. Population
    happens on the UI thread (import/patch); the engine only reads. */
class FixtureLibrary
{
public:
    FixtureLibrary() = default;

    /** Adds or replaces a fixture type (keyed by id()); returns its id. */
    juce::String add (FixtureType type);

    /** Imports a .gdtf archive into the library. Returns the id, or nullopt on failure. */
    std::optional<juce::String> importGdtf (const juce::File& gdtfArchive);

    [[nodiscard]] const FixtureType* find (const juce::String& id) const;
    [[nodiscard]] const DmxMode* findMode (const juce::String& id, const juce::String& modeName) const;

    [[nodiscard]] std::size_t size() const noexcept { return types.size(); }
    [[nodiscard]] bool empty() const noexcept { return types.empty(); }

    /** All types in import order. */
    [[nodiscard]] std::vector<const FixtureType*> all() const;

    /** Types grouped by manufacturer (sorted), for a library browser. */
    [[nodiscard]] std::map<juce::String, std::vector<const FixtureType*>> byManufacturer() const;

    void clear();

private:
    std::vector<std::unique_ptr<FixtureType>> types;
};
}
