#include "system_r/core/Engine.h"

#include "system_r/core/gdtf/FixtureType.h"

namespace systemr
{
namespace
{
    void renderFixture (const ShowDocument::Fixture& fixture,
                        const gdtf::DmxMode& mode,
                        std::uint8_t dimmerLevel,
                        Universe& out)
    {
        for (const auto& channel : mode.channels)
        {
            if (channel.isVirtual())
                continue;

            // Start from the GDTF default for this channel...
            auto value = channel.defaultValue.toChannelValue (channel.resolution());

            // ...then let the programmer's dimmer drive any Dimmer channel.
            if (channel.attribute.equalsIgnoreCase ("Dimmer"))
                value = gdtf::scaleLevel (dimmerLevel, channel.resolution());

            gdtf::writeChannelValue (out, fixture.address, channel, value);
        }
    }
}

void renderUniverse (const ShowDocument& show,
                     const gdtf::FixtureLibrary& library,
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

        if (const auto* mode = fixture.gdtfTypeId.isNotEmpty()
                                   ? library.findMode (fixture.gdtfTypeId, fixture.modeName)
                                   : nullptr)
        {
            renderFixture (fixture, *mode, level, out);
        }
        else
        {
            // Fallback: an untyped single-channel dimmer at the fixture's address.
            const int index = fixture.address - 1;
            if (index >= 0 && index < kUniverseSize)
                out[static_cast<std::size_t> (index)] = level;
        }
    }
}

Engine::Engine (ShowDocument& s, gdtf::FixtureLibrary& lib, Programmer& p, OutputManager& o)
    : show (s), library (lib), programmer (p), outputManager (o)
{
}

Engine::~Engine()
{
    stop();
}

void Engine::start (int rateHz)
{
    timerIntervalMs = juce::jmax (1, 1000 / rateHz);
    startTimer (timerIntervalMs);
}

void Engine::stop()
{
    stopTimer();
}

void Engine::pauseWhile (const std::function<void()>& edit)
{
    const bool wasRunning = isTimerRunning();
    if (wasRunning)
        stopTimer();

    edit();

    if (wasRunning)
        startTimer (timerIntervalMs);
}

void Engine::hiResTimerCallback()
{
    const auto& configs = outputManager.config();
    const double nowMs = juce::Time::getMillisecondCounterHiRes();

    UniverseSet& frames = snapshot.writeSlot();
    frames.resize (configs.size()); // no-op once the three slots are sized

    for (std::size_t i = 0; i < configs.size(); ++i)
    {
        renderUniverse (show, library, programmer, configs[i].index, frames[i]);
        outputManager.send (i, frames[i], nowMs);
    }

    snapshot.publish();
}
}
