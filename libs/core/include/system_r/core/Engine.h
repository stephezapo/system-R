#pragma once

#include <juce_core/juce_core.h>

#include "system_r/core/OutputManager.h"
#include "system_r/core/Programmer.h"
#include "system_r/core/ShowDocument.h"
#include "system_r/core/TripleBuffer.h"
#include "system_r/core/Universe.h"
#include "system_r/core/gdtf/FixtureLibrary.h"

#include <functional>
#include <vector>

namespace systemr
{
/** A rendered set of universes, one entry per configured output universe. */
using UniverseSet = std::vector<Universe>;

/** Pure render: resolve the show + programmer into one universe's slot values.

    For a GDTF-typed fixture this writes every channel's GDTF default, then applies the
    programmer dimmer to the fixture's Dimmer channel (coarse/fine per the mode's channel
    resolution). Fixtures with no GDTF type fall back to a single-channel dimmer. Kept free
    of threading/IO so the merge logic that grows here later stays unit-testable.
*/
void renderUniverse (const ShowDocument& show,
                     const gdtf::FixtureLibrary& library,
                     const Programmer& programmer,
                     int universeIndex,
                     Universe& out);

/** Fixed-rate engine: ticks on a dedicated high-resolution timer thread, renders every
    configured output universe, publishes a snapshot for the UI and transmits each via
    the OutputManager. Nothing here runs on the UI/message thread. */
class Engine : private juce::HighResolutionTimer
{
public:
    Engine (ShowDocument& show, gdtf::FixtureLibrary& library,
            Programmer& programmer, OutputManager& output);
    ~Engine() override;

    void start (int rateHz = 40);
    void stop();

    /** Runs @p edit with the engine tick paused, so structural edits to the show/library
        (patch, GDTF/MVR import) never race the render thread. Restarts the tick afterwards
        if it was running. Call on the message thread. */
    void pauseWhile (const std::function<void()>& edit);

    /** Snapshot of all rendered universes the UI polls to display live output. */
    [[nodiscard]] TripleBuffer<UniverseSet>& output() noexcept { return snapshot; }

private:
    void hiResTimerCallback() override;

    ShowDocument& show;
    gdtf::FixtureLibrary& library;
    Programmer& programmer;
    OutputManager& outputManager;
    TripleBuffer<UniverseSet> snapshot;
    int timerIntervalMs = 25;
};
}
