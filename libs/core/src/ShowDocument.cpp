#include "system_r/core/ShowDocument.h"

namespace systemr
{
namespace ids
{
    const juce::Identifier SHOW    { "SHOW" };
    const juce::Identifier PATCH   { "PATCH" };
    const juce::Identifier FIXTURE { "FIXTURE" };

    const juce::Identifier fid      { "fid" };
    const juce::Identifier name     { "name" };
    const juce::Identifier universe { "universe" };
    const juce::Identifier address  { "address" };
    const juce::Identifier channels { "channels" };
}

ShowDocument::ShowDocument()
    : show (ids::SHOW)
{
    show.appendChild (juce::ValueTree (ids::PATCH), nullptr);

    // Phase 1 default rig: one dimmer at universe 1 / address 1.
    addDimmer (1, "Dimmer 1", 1, 1);
}

ShowDocument::Fixture ShowDocument::addDimmer (int fid, const juce::String& name,
                                              int universe, int address)
{
    juce::ValueTree fixture (ids::FIXTURE);
    fixture.setProperty (ids::fid, fid, nullptr);
    fixture.setProperty (ids::name, name, nullptr);
    fixture.setProperty (ids::universe, universe, nullptr);
    fixture.setProperty (ids::address, address, nullptr);
    fixture.setProperty (ids::channels, 1, nullptr);

    show.getChildWithName (ids::PATCH).appendChild (fixture, &undo);

    return { fid, name, universe, address, 1 };
}

std::vector<ShowDocument::Fixture> ShowDocument::fixtures() const
{
    std::vector<Fixture> result;
    const auto patch = show.getChildWithName (ids::PATCH);

    for (const auto fixture : patch)
    {
        result.push_back ({
            (int) fixture.getProperty (ids::fid),
            fixture.getProperty (ids::name).toString(),
            (int) fixture.getProperty (ids::universe),
            (int) fixture.getProperty (ids::address),
            (int) fixture.getProperty (ids::channels)
        });
    }

    return result;
}
}
