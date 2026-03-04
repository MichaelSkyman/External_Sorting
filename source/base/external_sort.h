#pragma once
#include <string>
#include <vector>
#include <functional>

// Callback types for visualization
enum class SortPhase { READ, SORT, WRITE, MERGE };
using SortCallback = std::function<void(SortPhase phase, const std::vector<double>& data, int progress)>;

// ExternalSorter class declaration
class ExternalSorter {
public:
    ExternalSorter(size_t chunkSize);
    void sort(const std::string& inputFile, const std::string& outputFile);
    
    // Set callback for visualization
    void setCallback(SortCallback callback) { this->callback = callback; }
    
private:
    size_t chunkSize;
    SortCallback callback;
    
    std::vector<std::string> SplitandSort(const std::string& inputFile);
    std::string mergeSubset(const std::vector<std::string>& chunkFiles, size_t mergeIndex);
    void mergeSortedChunks(const std::vector<std::string>& chunkFiles, const std::string& outputFile);
    
    void notifyCallback(SortPhase phase, const std::vector<double>& data, int progress);
};