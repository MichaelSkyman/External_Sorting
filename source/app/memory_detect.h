#pragma once
#include <cstddef>

namespace MemoryDetect {

size_t getAvailableRAM();        // bytes
size_t getRecommendedChunkSize(); // number of double elements

}
