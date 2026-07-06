#include "system_r/core/gdtf/FixtureLibrary.h"

#include "system_r/core/gdtf/GdtfParser.h"

namespace systemr::gdtf
{
juce::String FixtureLibrary::add (FixtureType type)
{
    const auto id = type.id();

    for (auto& t : types)
    {
        if (t->id() == id)
        {
            *t = std::move (type); // replace in place; keeps the slot's address stable
            return id;
        }
    }

    types.push_back (std::make_unique<FixtureType> (std::move (type)));
    return id;
}

std::optional<juce::String> FixtureLibrary::importGdtf (const juce::File& gdtfArchive)
{
    auto type = parseGdtfFile (gdtfArchive);
    if (! type)
        return std::nullopt;
    return add (std::move (*type));
}

const FixtureType* FixtureLibrary::find (const juce::String& id) const
{
    for (const auto& t : types)
        if (t->id() == id)
            return t.get();
    return nullptr;
}

const DmxMode* FixtureLibrary::findMode (const juce::String& id, const juce::String& modeName) const
{
    if (const auto* t = find (id))
        return t->findMode (modeName);
    return nullptr;
}

std::vector<const FixtureType*> FixtureLibrary::all() const
{
    std::vector<const FixtureType*> out;
    out.reserve (types.size());
    for (const auto& t : types)
        out.push_back (t.get());
    return out;
}

std::map<juce::String, std::vector<const FixtureType*>> FixtureLibrary::byManufacturer() const
{
    std::map<juce::String, std::vector<const FixtureType*>> out;
    for (const auto& t : types)
        out[t->manufacturer].push_back (t.get());
    return out;
}

void FixtureLibrary::clear()
{
    types.clear();
}
}
