#pragma once

#include <string_view>

namespace systemr
{
/** Returns the semantic version of the system-R core, e.g. "0.0.1". */
[[nodiscard]] std::string_view coreVersion() noexcept;
}
