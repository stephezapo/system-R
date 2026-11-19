#include "system_r/core/mvr/MvrImporter.h"

#include "system_r/core/gdtf/GdtfParser.h"

#include <map>
#include <memory>

namespace systemr::mvr
{
namespace
{
    juce::String childText (const juce::XmlElement& parent, juce::StringRef tag)
    {
        if (auto* c = parent.getChildByName (tag))
            return c->getAllSubText().trim();
        return {};
    }

    void parseMatrixPosition (const juce::String& matrix, float& x, float& y, float& z)
    {
        juce::StringArray groups;
        int i = 0;
        while ((i = matrix.indexOfChar (i, '{')) >= 0)
        {
            const int end = matrix.indexOfChar (i, '}');
            if (end < 0)
                break;
            groups.add (matrix.substring (i + 1, end));
            i = end + 1;
        }

        if (groups.isEmpty())
            return;

        const auto comps = juce::StringArray::fromTokens (groups[groups.size() - 1], ",", "");
        if (comps.size() >= 3)
        {
            x = comps[0].getFloatValue();
            y = comps[1].getFloatValue();
            z = comps[2].getFloatValue();
        }
    }

    void parseFixture (const juce::XmlElement& xf, const juce::String& layer, MvrScene& scene)
    {
        MvrFixture f;
        f.name     = xf.getStringAttribute ("name");
        f.uuid     = xf.getStringAttribute ("uuid");
        f.layer    = layer;
        f.gdtfSpec = childText (xf, "GDTFSpec");
        f.mode     = childText (xf, "GDTFMode");
        f.fixtureId = childText (xf, "FixtureID").getIntValue();

        if (auto* addresses = xf.getChildByName ("Addresses"))
        {
            if (auto* addr = addresses->getChildByName ("Address"))
            {
                f.dmxBreak = addr->getIntAttribute ("break", 1);
                const int absolute = addr->getAllSubText().trim().getIntValue();
                f.universe = universeFromAbsolute (absolute);
                f.address  = addressFromAbsolute (absolute);
            }
        }

        parseMatrixPosition (childText (xf, "Matrix"), f.x, f.y, f.z);
        scene.fixtures.push_back (std::move (f));
    }

    // Recurses a <ChildList> collecting fixtures (also inside nested GroupObjects).
    void collectFixtures (const juce::XmlElement& childList, const juce::String& layer, MvrScene& scene)
    {
        for (auto* child : childList.getChildIterator())
        {
            if (child->hasTagName ("Fixture"))
                parseFixture (*child, layer, scene);
            else if (auto* nested = child->getChildByName ("ChildList"))
                collectFixtures (*nested, layer, scene);
        }
    }

    int findEntryIndex (juce::ZipFile& zip, const juce::String& filename)
    {
        for (int i = 0; i < zip.getNumEntries(); ++i)
        {
            const auto* entry = zip.getEntry (i);
            if (entry != nullptr
                && (entry->filename == filename || entry->filename.endsWithIgnoreCase (filename)))
                return i;
        }
        return -1;
    }

    std::optional<MvrScene> parseSceneFromZip (juce::ZipFile& zip)
    {
        const int index = findEntryIndex (zip, "GeneralSceneDescription.xml");
        if (index < 0)
            return std::nullopt;

        std::unique_ptr<juce::InputStream> stream (zip.createStreamForEntry (index));
        if (stream == nullptr)
            return std::nullopt;

        return parseSceneXml (stream->readEntireStreamAsString());
    }

    // Imports each referenced GDTF from the archive into the library and resolves type ids.
    void resolveGdtfs (juce::ZipFile& zip, MvrScene& scene, gdtf::FixtureLibrary& library)
    {
        std::map<juce::String, juce::String> specToId; // gdtfSpec -> library id

        for (auto& f : scene.fixtures)
        {
            if (f.gdtfSpec.isEmpty())
                continue;

            auto cached = specToId.find (f.gdtfSpec);
            if (cached != specToId.end())
            {
                f.gdtfTypeId = cached->second;
                continue;
            }

            const int index = findEntryIndex (zip, f.gdtfSpec);
            if (index < 0)
                continue;

            std::unique_ptr<juce::InputStream> stream (zip.createStreamForEntry (index));
            if (stream == nullptr)
                continue;

            juce::MemoryOutputStream raw;
            raw.writeFromInputStream (*stream, -1);

            if (auto type = gdtf::parseGdtfData (raw.getData(), raw.getDataSize()))
            {
                const auto id = library.add (std::move (*type));
                specToId[f.gdtfSpec] = id;
                f.gdtfTypeId = id;
            }
        }
    }

    std::optional<MvrScene> importFromZip (juce::ZipFile& zip, gdtf::FixtureLibrary& library)
    {
        auto scene = parseSceneFromZip (zip);
        if (! scene)
            return std::nullopt;

        resolveGdtfs (zip, *scene, library);
        return scene;
    }
}

int universeFromAbsolute (int absolute) noexcept
{
    if (absolute < 1)
        return 1;
    return (absolute - 1) / kUniverseSize + 1;
}

int addressFromAbsolute (int absolute) noexcept
{
    if (absolute < 1)
        return 1;
    return (absolute - 1) % kUniverseSize + 1;
}

std::optional<MvrScene> parseSceneXml (const juce::String& xml)
{
    auto root = juce::XmlDocument::parse (xml);
    if (root == nullptr || ! root->hasTagName ("GeneralSceneDescription"))
        return std::nullopt;

    auto* scene = root->getChildByName ("Scene");
    if (scene == nullptr)
        return std::nullopt;

    MvrScene result;
    if (auto* layers = scene->getChildByName ("Layers"))
    {
        for (auto* layer : layers->getChildWithTagNameIterator ("Layer"))
        {
            const auto layerName = layer->getStringAttribute ("name");
            result.layers.add (layerName);
            if (auto* childList = layer->getChildByName ("ChildList"))
                collectFixtures (*childList, layerName, result);
        }
    }

    return result;
}

std::optional<MvrScene> importMvrFile (const juce::File& mvrArchive, gdtf::FixtureLibrary& library)
{
    juce::ZipFile zip (mvrArchive);
    return importFromZip (zip, library);
}

std::optional<MvrScene> importMvrData (const void* data, size_t numBytes, gdtf::FixtureLibrary& library)
{
    juce::MemoryInputStream mem (data, numBytes, false);
    juce::ZipFile zip (mem);
    return importFromZip (zip, library);
}

int applyToPatch (ShowDocument& show, const gdtf::FixtureLibrary& library,
                  const MvrScene& scene, int firstFid)
{
    int fid = firstFid;
    int added = 0;

    for (const auto& f : scene.fixtures)
    {
        const auto* mode = library.findMode (f.gdtfTypeId, f.mode);
        const int channelCount = mode != nullptr ? juce::jmax (1, mode->channelCount()) : 1;

        show.addFixture (fid++, f.name, f.gdtfTypeId, f.mode, f.universe, f.address, channelCount);
        ++added;
    }

    return added;
}
}
