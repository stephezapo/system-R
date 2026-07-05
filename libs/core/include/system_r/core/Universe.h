#pragma once

#include <array>
#include <cstdint>

namespace systemr
{
/** Number of DMX slots in one universe (excluding the start code). */
inline constexpr int kUniverseSize = 512;

/** One universe of DMX512 slot values (0..255), index 0 == DMX address 1. */
using Universe = std::array<std::uint8_t, kUniverseSize>;
}
