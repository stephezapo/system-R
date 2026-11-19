#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "BinaryData.h"

namespace systemr::icons
{
/** Lucide icon font (lucide-static 1.76.0, ISC licence — see resources/fonts/LICENSE-lucide.txt).

    Codepoints come from the package's font/info.json and are NOT stable across Lucide
    releases: when updating lucide.ttf, re-check every value below. */
enum class Glyph : juce::juce_wchar
{
    folder   = 0xe0d7,
    monitor  = 0xe11d,
    settings = 0xe154,
};

/** The embedded Lucide typeface, loaded once on first use. */
inline juce::Typeface::Ptr typeface()
{
    static const juce::Typeface::Ptr tf =
        juce::Typeface::createSystemTypefaceFor (BinaryData::lucide_ttf,
                                                 static_cast<size_t> (BinaryData::lucide_ttfSize));
    return tf;
}

/** Draws a Lucide glyph centred in `area`, `size` pixels tall. */
inline void draw (juce::Graphics& g, Glyph glyph, juce::Rectangle<float> area, float size)
{
    g.setFont (juce::FontOptions (typeface()).withHeight (size));
    g.drawText (juce::String::charToString (static_cast<juce::juce_wchar> (glyph)),
                area, juce::Justification::centred, false);
}
} // namespace systemr::icons
