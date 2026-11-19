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

/** The glyph's outline as a path (font units at height 1; position arbitrary). Lucide
    icons sit on a 24x24 grid; use the path's bounds to locate parts of an icon. */
inline juce::Path outline (Glyph glyph)
{
    juce::GlyphArrangement ga;
    ga.addLineOfText (juce::FontOptions (typeface()).withHeight (1.0f),
                      juce::String::charToString (static_cast<juce::juce_wchar> (glyph)), 0.0f, 0.0f);
    juce::Path p;
    ga.createPath (p);
    return p;
}

/** Draws a Lucide glyph centred in `area`, `size` pixels tall. */
inline void draw (juce::Graphics& g, Glyph glyph, juce::Rectangle<float> area, float size)
{
    g.setFont (juce::FontOptions (typeface()).withHeight (size));
    g.drawText (juce::String::charToString (static_cast<juce::juce_wchar> (glyph)),
                area, juce::Justification::centred, false);
}
} // namespace systemr::icons
