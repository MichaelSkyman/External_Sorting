#pragma once
#include <cstddef>

/// @brief Utilities for querying system memory and deriving safe chunk sizes.
namespace MemoryDetect {

/// @brief Returns the amount of currently available physical RAM in bytes.
size_t getAvailableRAM();

/// @brief Returns the recommended number of @c double elements per sort chunk.
///
/// Uses approximately 25% of available RAM to leave headroom for the OS
/// and other processes.
size_t getRecommendedChunkSize();

} // namespace MemoryDetect
