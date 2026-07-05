#include "system_r/core/Engine.h"

namespace systemr
{
void renderUniverse (const ShowDocument& show,
                     const Programmer& programmer,
                     int universeIndex,
                     Universe& out)
{
    out.fill (0);

    const auto level = programmer.dimmerLevel();

    for (const auto& fixture : show.fixtures())
    {
        if (fixture.universe != universeIndex)
            continue;

        const int index = fixture.address - 1; // DMX address is 1-based
        if (index >= 0 && index < kUniverseSize)
            out[static_cast<std::size_t> (index)] = level;
    }
}

Engine::Engine (ShowDocument& s, Programmer& p, SacnSender& snd)
    : show (s), programmer (p), sender (snd)
{
}

Engine::~Engine()
{
    stop();
}

void Engine::start (int rateHz)
{
    startTimer (juce::jmax (1, 1000 / rateHz));
}

void Engine::stop()
{
    stopTimer();
}

void Engine::hiResTimerCallback()
{
    Universe& frame = snapshot.writeSlot();

    renderUniverse (show, programmer, sender.universe(), frame);
    sender.send (frame);
    snapshot.publish();
}
}
