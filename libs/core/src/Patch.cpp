#include "system_r/core/Patch.h"

#include "system_r/core/Universe.h"

namespace systemr::patch
{
bool wouldOverlap (const std::vector<ShowDocument::Fixture>& existing,
                   int universe, int address, int channelCount, int ignoreFid)
{
    for (const auto& f : existing)
    {
        if (f.fid == ignoreFid || f.universe != universe)
            continue;
        if (rangesOverlap (address, channelCount, f.address, f.channelCount))
            return true;
    }
    return false;
}

std::set<int> overlappingFixtures (const std::vector<ShowDocument::Fixture>& fixtures)
{
    std::set<int> conflicts;
    for (std::size_t i = 0; i < fixtures.size(); ++i)
    {
        for (std::size_t j = i + 1; j < fixtures.size(); ++j)
        {
            const auto& a = fixtures[i];
            const auto& b = fixtures[j];
            if (a.universe == b.universe
                && rangesOverlap (a.address, a.channelCount, b.address, b.channelCount))
            {
                conflicts.insert (a.fid);
                conflicts.insert (b.fid);
            }
        }
    }
    return conflicts;
}

int firstFreeAddress (const std::vector<ShowDocument::Fixture>& fixtures,
                      int universe, int channelCount)
{
    if (channelCount <= 0 || channelCount > kUniverseSize)
        return -1;

    for (int address = 1; address + channelCount - 1 <= kUniverseSize; ++address)
    {
        if (! wouldOverlap (fixtures, universe, address, channelCount))
            return address;
    }
    return -1;
}
}
