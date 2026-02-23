#pragma once
#include <string>

enum class ChunkMode {
    AUTO,
    MANUAL
};

class AppConfig {
public:
    void setInputFile(const std::string& path);
    void setOutputFile(const std::string& path);

    void setChunkMode(ChunkMode mode);
    void setManualChunkSize(size_t size);

    std::string getInputFile() const;
    std::string getOutputFile() const;

    size_t getChunkSize() const;

private:
    std::string inputFile;
    std::string outputFile;

    ChunkMode chunkMode = ChunkMode::AUTO;
    size_t manualChunkSize = 0;
};
