#include "system_r/core/gdtf/GdtfParser.h"

#include <memory>

namespace systemr::gdtf
{
namespace
{
    std::vector<int> parseOffsets (const juce::String& offset)
    {
        std::vector<int> result;
        const auto s = offset.trim();
        if (s.isEmpty() || s.equalsIgnoreCase ("None"))
            return result; // virtual channel

        for (const auto& tok : juce::StringArray::fromTokens (s, ",", ""))
        {
            const auto t = tok.trim();
            if (t.containsOnly ("0123456789") && t.isNotEmpty())
                result.push_back (t.getIntValue());
        }
        return result;
    }

    ChannelFunction parseChannelFunction (const juce::XmlElement& cf, int channelBytes)
    {
        ChannelFunction fn;
        fn.attribute    = cf.getStringAttribute ("Attribute");
        fn.name         = cf.getStringAttribute ("Name");
        fn.dmxFrom      = DmxValue::parse (cf.getStringAttribute ("DMXFrom", "0/1"), channelBytes);
        fn.defaultValue = DmxValue::parse (cf.getStringAttribute ("Default", "0/1"), channelBytes);
        fn.physicalFrom = cf.getDoubleAttribute ("PhysicalFrom", 0.0);
        fn.physicalTo   = cf.getDoubleAttribute ("PhysicalTo", 1.0);
        return fn;
    }

    DmxChannel parseChannel (const juce::XmlElement& xc)
    {
        DmxChannel ch;
        ch.dmxBreak = xc.getIntAttribute ("DMXBreak", 1);
        ch.offsets  = parseOffsets (xc.getStringAttribute ("Offset"));
        ch.geometry = xc.getStringAttribute ("Geometry");

        const int bytes = ch.resolution();

        // LogicalChannel carries the human attribute; ChannelFunctions carry the ranges.
        for (auto* lc : xc.getChildWithTagNameIterator ("LogicalChannel"))
        {
            if (ch.attribute.isEmpty())
                ch.attribute = lc->getStringAttribute ("Attribute");

            for (auto* cf : lc->getChildWithTagNameIterator ("ChannelFunction"))
                ch.functions.push_back (parseChannelFunction (*cf, bytes));
        }

        if (ch.attribute.isEmpty() && ! ch.functions.empty())
            ch.attribute = ch.functions.front().attribute;

        // Default: GDTF 1.0 put it on the channel; 1.1+ on the first ChannelFunction.
        if (xc.hasAttribute ("Default"))
            ch.defaultValue = DmxValue::parse (xc.getStringAttribute ("Default"), bytes);
        else if (! ch.functions.empty())
            ch.defaultValue = ch.functions.front().defaultValue;
        else
            ch.defaultValue = DmxValue { 0, bytes };

        if (xc.hasAttribute ("Highlight") && ! xc.getStringAttribute ("Highlight").equalsIgnoreCase ("None"))
        {
            ch.hasHighlight = true;
            ch.highlight    = DmxValue::parse (xc.getStringAttribute ("Highlight"), bytes);
        }

        return ch;
    }

    DmxMode parseMode (const juce::XmlElement& xm)
    {
        DmxMode mode;
        mode.name     = xm.getStringAttribute ("Name");
        mode.geometry = xm.getStringAttribute ("Geometry");

        if (auto* channels = xm.getChildByName ("DMXChannels"))
            for (auto* xc : channels->getChildWithTagNameIterator ("DMXChannel"))
                mode.channels.push_back (parseChannel (*xc));

        return mode;
    }

    std::optional<FixtureType> parseGdtfFromZip (juce::ZipFile& zip)
    {
        int index = -1;
        for (int i = 0; i < zip.getNumEntries(); ++i)
        {
            const auto* entry = zip.getEntry (i);
            if (entry != nullptr && entry->filename.endsWithIgnoreCase ("description.xml"))
            {
                index = i;
                break;
            }
        }
        if (index < 0)
            return std::nullopt;

        std::unique_ptr<juce::InputStream> stream (zip.createStreamForEntry (index));
        if (stream == nullptr)
            return std::nullopt;

        return parseDescriptionXml (stream->readEntireStreamAsString());
    }
}

std::optional<FixtureType> parseGdtfRoot (const juce::XmlElement& gdtfRoot)
{
    const juce::XmlElement* ft = gdtfRoot.hasTagName ("FixtureType")
                                     ? &gdtfRoot
                                     : gdtfRoot.getChildByName ("FixtureType");
    if (ft == nullptr)
        return std::nullopt;

    FixtureType type;
    type.manufacturer  = ft->getStringAttribute ("Manufacturer");
    type.name          = ft->getStringAttribute ("Name");
    type.shortName     = ft->getStringAttribute ("ShortName");
    type.fixtureTypeId = ft->getStringAttribute ("FixtureTypeID");
    type.gdtfVersion   = gdtfRoot.getStringAttribute ("DataVersion");

    if (auto* modes = ft->getChildByName ("DMXModes"))
        for (auto* xm : modes->getChildWithTagNameIterator ("DMXMode"))
            type.modes.push_back (parseMode (*xm));

    if (type.name.isEmpty() && type.modes.empty())
        return std::nullopt; // not a usable fixture type

    return type;
}

std::optional<FixtureType> parseDescriptionXml (const juce::String& xml)
{
    auto root = juce::XmlDocument::parse (xml);
    if (root == nullptr)
        return std::nullopt;
    return parseGdtfRoot (*root);
}

std::optional<FixtureType> parseGdtfFile (const juce::File& gdtfArchive)
{
    juce::ZipFile zip (gdtfArchive);
    return parseGdtfFromZip (zip);
}

std::optional<FixtureType> parseGdtfData (const void* data, size_t numBytes)
{
    juce::MemoryInputStream mem (data, numBytes, false);
    juce::ZipFile zip (&mem, false);
    return parseGdtfFromZip (zip);
}
}
