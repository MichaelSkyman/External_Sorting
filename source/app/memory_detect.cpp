#include "memory_detect.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/sysinfo.h>
#endif

namespace MemoryDetect {

size_t getAvailableRAM() {
#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullAvailPhys;
#else
    struct sysinfo info;
    sysinfo(&info);
    return info.freeram * info.mem_unit;
#endif
}

size_t getRecommendedChunkSize() {
    size_t ram = getAvailableRAM();

    // Use only ~1/4 RAM for safety
    size_t usable = ram / 4;

    return usable / sizeof(double);
}

}
