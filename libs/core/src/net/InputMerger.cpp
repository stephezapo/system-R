#include "system_r/core/net/InputMerger.h"

namespace systemr
{
InputMerger::Source* InputMerger::find (std::vector<Source>& list, const juce::Uuid& id) noexcept
{
    for (auto& s : list)
        if (s.id == id)
            return &s;
    return nullptr;
}

void InputMerger::ingest (int universe,
                          const juce::Uuid& sourceId,
                          const juce::String& name,
                          std::uint8_t priority,
                          bool isArtNet,
                          const Universe& slots,
                          double nowMs)
{
    const juce::ScopedLock sl (lock);

    auto& list = universes[universe];
    Source* s = find (list, sourceId);
    if (s == nullptr)
    {
        list.push_back (Source {});
        s = &list.back();
        s->id = sourceId;
    }

    s->name       = name;
    s->priority   = priority;
    s->isArtNet   = isArtNet;
    s->lastSeenMs = nowMs;
    s->slots      = slots;
}

void InputMerger::removeSource (int universe, const juce::Uuid& sourceId)
{
    const juce::ScopedLock sl (lock);

    auto it = universes.find (universe);
    if (it == universes.end())
        return;

    auto& list = it->second;
    for (auto i = list.begin(); i != list.end(); ++i)
    {
        if (i->id == sourceId)
        {
            list.erase (i);
            break;
        }
    }
}

Universe InputMerger::merged (int universe, double nowMs) const
{
    const juce::ScopedLock sl (lock);

    Universe result {};

    auto it = universes.find (universe);
    if (it == universes.end())
        return result;

    const auto& list = it->second;

    // Highest priority among live sources wins the universe outright.
    int winningPriority = -1;
    for (const auto& s : list)
        if (s.live (nowMs))
            winningPriority = juce::jmax (winningPriority, static_cast<int> (s.priority));

    if (winningPriority < 0)
        return result; // no live source

    // Among the winners, merge HTP per slot.
    for (const auto& s : list)
    {
        if (! s.live (nowMs) || static_cast<int> (s.priority) != winningPriority)
            continue;

        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] = juce::jmax (result[i], s.slots[i]);
    }

    return result;
}

std::vector<InputMerger::SourceInfo> InputMerger::sources (int universe, double nowMs) const
{
    const juce::ScopedLock sl (lock);

    std::vector<SourceInfo> out;
    auto it = universes.find (universe);
    if (it == universes.end())
        return out;

    for (const auto& s : it->second)
    {
        if (! s.live (nowMs))
            continue;
        out.push_back (SourceInfo { s.id, s.name, s.priority, s.isArtNet, s.lastSeenMs });
    }
    return out;
}

bool InputMerger::capture (int universe, const juce::Uuid& sourceId,
                           Universe& out, double nowMs) const
{
    const juce::ScopedLock sl (lock);

    auto it = universes.find (universe);
    if (it == universes.end())
        return false;

    for (const auto& s : it->second)
    {
        if (s.id == sourceId && s.live (nowMs))
        {
            out = s.slots;
            return true;
        }
    }
    return false;
}

std::vector<int> InputMerger::liveUniverses (double nowMs) const
{
    const juce::ScopedLock sl (lock);

    std::vector<int> out;
    for (const auto& [universe, list] : universes)
    {
        for (const auto& s : list)
        {
            if (s.live (nowMs))
            {
                out.push_back (universe);
                break;
            }
        }
    }
    return out;
}

void InputMerger::prune (double nowMs)
{
    const juce::ScopedLock sl (lock);

    for (auto it = universes.begin(); it != universes.end();)
    {
        auto& list = it->second;
        for (auto s = list.begin(); s != list.end();)
            s = s->live (nowMs) ? std::next (s) : list.erase (s);

        it = list.empty() ? universes.erase (it) : std::next (it);
    }
}
}
