#pragma once

#include <cstdint>
#include <limits>

namespace Config {
    constexpr uint64_t INF = std::numeric_limits<uint64_t>::max() / 4;
    constexpr uint64_t INF_LIKE = std::numeric_limits<uint64_t>::max() / 2;
    constexpr int DEFAULT_THREADS = 1;
    constexpr int MAX_THREADS = 64;
}
