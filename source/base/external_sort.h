#pragma once
#include <string>
#include <vector>
#include <functional>

/** @brief Phases of external sort reported to the visualization callback. */
enum class SortPhase {
    READ,   ///< Reading a data block from disk into RAM.
    SORT,   ///< Sorting the block in RAM.
    WRITE,  ///< Writing the sorted block to a temporary run file.
    MERGE   ///< Merging sorted runs into the final output.
};

/**
 * @brief Callback invoked at each sort phase for live visualization.
 *
 * @param phase    Current sort phase.
 * @param data     Subset of values being processed (up to 20 elements).
 * @param progress Overall progress percentage (0–100).
 */
using SortCallback = std::function<void(SortPhase phase, const std::vector<double>& data, int progress)>;

/**
 * @brief Performs external merge-sort on large binary files.
 *
 * Splits the input into sorted in-memory chunks sized to available RAM,
 * then performs a multi-pass k-way heap merge to produce fully sorted output.
 */
class ExternalSorter {
public:
    /// @brief Constructs the sorter with the given chunk size.
    /// @param chunkSize Number of double elements per in-memory chunk.
    ExternalSorter(size_t chunkSize);

    /// @brief Sorts @p inputFile and writes the result to @p outputFile.
    /// @param inputFile  Path to the binary input file (raw doubles).
    /// @param outputFile Path to the binary output file to be created.
    void sort(const std::string& inputFile, const std::string& outputFile);

    /// @brief Sets the progress callback used during visualization.
    /// @param callback Function called at each READ / SORT / WRITE / MERGE phase.
    void setCallback(SortCallback callback) { this->callback = callback; }

private:
    size_t chunkSize;       ///< Maximum number of elements per in-memory chunk.
    SortCallback callback;  ///< Optional visualization callback (may be null).

    std::vector<std::string> SplitandSort(const std::string& inputFile);
    std::string mergeSubset(const std::vector<std::string>& chunkFiles, size_t mergeIndex);
    void mergeSortedChunks(const std::vector<std::string>& chunkFiles, const std::string& outputFile);

    void notifyCallback(SortPhase phase, const std::vector<double>& data, int progress);
};