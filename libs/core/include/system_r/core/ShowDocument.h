#pragma once

#include <juce_data_structures/juce_data_structures.h>

#include <vector>

namespace systemr
{
/** Minimal, ValueTree-backed show document.

    Phase 1 models only what the walking skeleton needs: a patch of fixtures, each a
    single-channel dimmer. The tree is the seed of the full document (presets, cues,
    universes, settings) and already sits under an UndoManager. Structural access is
    exposed through small accessors so callers never touch raw Identifiers.
*/
class ShowDocument
{
public:
    /** A patched fixture, resolved out of the tree for the engine to render. */
    struct Fixture
    {
        int fid = 0;
        juce::String name;
        int universe = 1;         // 1-based universe index
        int address = 1;          // 1-based DMX start address
        int channelCount = 1;
        juce::String gdtfTypeId;  // library id; empty => plain single-channel dimmer
        juce::String modeName;    // selected DMX mode within the type
    };

    ShowDocument();

    /** Adds a single-channel dimmer to the patch (no GDTF type) and returns it. */
    Fixture addDimmer (int fid, const juce::String& name, int universe, int address);

    /** Adds a GDTF-typed fixture to the patch and returns it. */
    Fixture addFixture (int fid, const juce::String& name,
                        const juce::String& gdtfTypeId, const juce::String& modeName,
                        int universe, int address, int channelCount);

    /** Removes the fixture with the given fid; returns true if one was removed. */
    bool removeFixture (int fid);

    /** Every patched fixture, in patch order. */
    [[nodiscard]] std::vector<Fixture> fixtures() const;

    [[nodiscard]] juce::ValueTree& state() noexcept              { return show; }
    [[nodiscard]] const juce::ValueTree& state() const noexcept  { return show; }
    [[nodiscard]] juce::UndoManager& undoManager() noexcept      { return undo; }

private:
    juce::UndoManager undo;
    juce::ValueTree show;
};
}
