#pragma once

#include "system_r/core/ShowDocument.h"

#include <set>
#include <vector>

namespace systemr::patch
{
/** Half-open channel footprint [address, address + channelCount) on a universe. */
[[nodiscard]] inline bool rangesOverlap (int aStart, int aCount, int bStart, int bCount) noexcept
{
    return aStart < bStart + bCount && bStart < aStart + aCount;
}

/** True if a fixture of @p channelCount at @p universe/@p address would collide with any
    of @p existing (same universe, overlapping channel range). @p ignoreFid is skipped so a
    fixture can be re-validated against the patch without matching itself. */
[[nodiscard]] bool wouldOverlap (const std::vector<ShowDocument::Fixture>& existing,
                                 int universe, int address, int channelCount,
                                 int ignoreFid = -1);

/** fids of every fixture that overlaps at least one other fixture in the patch. */
[[nodiscard]] std::set<int> overlappingFixtures (const std::vector<ShowDocument::Fixture>& fixtures);

/** First free start address on @p universe that fits @p channelCount without overlap, or
    -1 if the universe cannot hold it. */
[[nodiscard]] int firstFreeAddress (const std::vector<ShowDocument::Fixture>& fixtures,
                                    int universe, int channelCount);
}
