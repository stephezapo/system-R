#pragma once

#include <juce_core/juce_core.h>

namespace systemr::test
{
/** A small but representative GDTF description.xml: an 8-bit Dimmer, a 16-bit Pan
    (coarse/fine), and a virtual (no-offset) channel. Used across the GDTF/engine tests. */
inline juce::String sampleGdtfXml()
{
    return R"(<?xml version="1.0" encoding="UTF-8"?>
<GDTF DataVersion="1.2">
  <FixtureType Manufacturer="Acme" Name="Test Spot" ShortName="TSpot"
               FixtureTypeID="11111111-2222-3333-4444-555555555555">
    <DMXModes>
      <DMXMode Name="Standard" Geometry="Base">
        <DMXChannels>
          <DMXChannel DMXBreak="1" Offset="1" Geometry="Base" Highlight="255/1">
            <LogicalChannel Attribute="Dimmer">
              <ChannelFunction Attribute="Dimmer" Name="Dimmer" DMXFrom="0/1" Default="0/1"
                               PhysicalFrom="0" PhysicalTo="100"/>
            </LogicalChannel>
          </DMXChannel>
          <DMXChannel DMXBreak="1" Offset="2,3" Geometry="Yoke">
            <LogicalChannel Attribute="Pan">
              <ChannelFunction Attribute="Pan" Name="Pan" DMXFrom="0/1" Default="32768/2"
                               PhysicalFrom="-270" PhysicalTo="270"/>
            </LogicalChannel>
          </DMXChannel>
          <DMXChannel DMXBreak="1" Offset="None">
            <LogicalChannel Attribute="Macro">
              <ChannelFunction Attribute="Macro" Name="Macro"/>
            </LogicalChannel>
          </DMXChannel>
        </DMXChannels>
      </DMXMode>
    </DMXModes>
  </FixtureType>
</GDTF>)";
}
}
