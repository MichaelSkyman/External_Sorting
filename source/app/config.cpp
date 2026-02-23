#include "config.h"
#include "memory_detect.h"

void AppConfig::setInputFile(const std::string& path) {
    inputFile = path;
}

void AppConfig::setOutputFile(const std::string& path) {
    outputFile = path;
}

void AppConfig::setChunkMode(ChunkMode mode) {
    chunkMode = mode;
}

void AppConfig::setManualChunkSize(size_t size) {
    manualChunkSize = size;
}

std::string AppConfig::getInputFile() const {
    return inputFile;
}

std::string AppConfig::getOutputFile() const {
    return outputFile;
}

size_t AppConfig::getChunkSize() const {
    if (chunkMode == ChunkMode::AUTO)
        return MemoryDetect::getRecommendedChunkSize();
    return manualChunkSize;
}
