#pragma once
#include <string>
#include <vector>

// ExternalSorter class declaration
class ExternalSorter {
public:
    ExternalSorter(size_t chunkSize);
    void sort(const std::string& inputFile, const std::string& outputFile);
private:
    size_t chunkSize;
    
    std::vector<std::string> SplitandSort(const std::string& inputFile);
    std::string mergeSubset(const std::vector<std::string>& chunkFiles, size_t mergeIndex);
    void mergeSortedChunks(const std::vector<std::string>& chunkFiles, const std::string& outputFile);
};