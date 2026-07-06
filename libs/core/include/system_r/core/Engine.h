#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/OutputManager.h"
#include "system_r/core/Programmer.h"
#include "system_r/core/ShowDocument.h"
#include "system_r/core/TripleBuffer.h"
#include "system_r/core/Universe.h"

#include <vector>

namespace systemr
{
/** A rendered set of universes, one entry per configured output universe. */
using UniverseSet = std::vector<Universe>;

/** Pure render: resolve the show + programmer into one universe's slot values.

    Phase 1/2 value stack = programmer dimmer over a home value of 0. Kept free of
    threading/IO so it can be unit-tested directly and so the merge logic that grows
    here later (playback, presets, external input) stays testable in isolation.
*/
void renderUniverse (const ShowDocument& show,
                     const Programmer& programmer,
                     int universeIndex,
                     Universe& out);

/** Fixed-rate engine: ticks on a dedicated high-resolution timer thread, renders every
    configured output universe, publishes a snapshot for the UI and transmits each via
    the OutputManager. Nothing here runs on the UI/message thread. */
class Engine : private juce::HighResolutionTimer
{
public:
    Engine (ShowDocument& show, Programmer& programmer, OutputManager& output);
    ~Engine() override;

    void start (int rateHz = 40);
    void stop();

    /** Snapshot of all rendered universes the UI polls to display live output. */
    [[nodiscard]] TripleBuffer<UniverseSet>& output() noexcept { return snapshot; }

private:
    void hiResTimerCallback() override;

    ShowDocument& show;
    Programmer& programmer;
    OutputManager& outputManager;
    TripleBuffer<UniverseSet> snapshot;
};
}
