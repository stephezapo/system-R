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
    const juce::Identifier gdtfType { "gdtfType" };
    const juce::Identifier mode     { "mode" };
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
    return addFixture (fid, name, {}, {}, universe, address, 1);
}

ShowDocument::Fixture ShowDocument::addFixture (int fid, const juce::String& name,
                                               const juce::String& gdtfTypeId, const juce::String& modeName,
                                               int universe, int address, int channelCount)
{
    juce::ValueTree fixture (ids::FIXTURE);
    fixture.setProperty (ids::fid, fid, nullptr);
    fixture.setProperty (ids::name, name, nullptr);
    fixture.setProperty (ids::universe, universe, nullptr);
    fixture.setProperty (ids::address, address, nullptr);
    fixture.setProperty (ids::channels, channelCount, nullptr);
    if (gdtfTypeId.isNotEmpty())
    {
        fixture.setProperty (ids::gdtfType, gdtfTypeId, nullptr);
        fixture.setProperty (ids::mode, modeName, nullptr);
    }

    show.getChildWithName (ids::PATCH).appendChild (fixture, &undo);

    return { fid, name, universe, address, channelCount, gdtfTypeId, modeName };
}

bool ShowDocument::removeFixture (int fid)
{
    auto patch = show.getChildWithName (ids::PATCH);
    for (int i = patch.getNumChildren() - 1; i >= 0; --i)
    {
        const auto child = patch.getChild (i);
        if ((int) child.getProperty (ids::fid) == fid)
        {
            patch.removeChild (i, &undo);
            return true;
        }
    }
    return false;
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
            (int) fixture.getProperty (ids::channels),
            fixture.getProperty (ids::gdtfType).toString(),
            fixture.getProperty (ids::mode).toString()
        });
    }

    return result;
}
}
