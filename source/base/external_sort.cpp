#include "external_sort.h"
#include <fstream>
#include <algorithm>
#include <queue>
#include <cstdio>

ExternalSorter::ExternalSorter(size_t size)
    : chunkSize(size) {}

std::vector<std::string> ExternalSorter::SplitandSort(const std::string& inputFile) {
    std::vector<std::string> chunkFiles;
    std::ifstream in(inputFile, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Could not open input file");
    }

    std::vector<double> buffer;
    buffer.reserve(chunkSize);
    size_t chunkIndex = 0;

    double number;
    while (in.read(reinterpret_cast<char*>(&number), sizeof(double))) {
        buffer.push_back(number);
        if (buffer.size() == chunkSize) {
            std::sort(buffer.begin(), buffer.end());
            std::string chunkFile = "chunk_" + std::to_string(chunkIndex++) + ".bin";
            std::ofstream out(chunkFile, std::ios::binary);
            out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(double));
            out.close();
            chunkFiles.push_back(chunkFile);
            buffer.clear();
        }
    }

    // Handle remaining numbers
    if (!buffer.empty()) {
        std::sort(buffer.begin(), buffer.end());
        std::string chunkFile = "chunk_" + std::to_string(chunkIndex++) + ".bin";
        std::ofstream out(chunkFile, std::ios::binary);
        out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(double));
        out.close();
        chunkFiles.push_back(chunkFile);
    }

    return chunkFiles;
}

// Heap element: stores the value and which chunk file it came from
struct HeapNode {
    double value;
    size_t chunkIndex;
    
    // Min-heap: smallest value has highest priority
    bool operator>(const HeapNode& other) const {
        return value > other.value;
    }
};

// Maximum number of files to merge at once (to avoid file handle limits)
static const size_t MAX_MERGE_FILES = 100;

// Merge a subset of chunk files into a single output file
std::string ExternalSorter::mergeSubset(const std::vector<std::string>& chunkFiles, size_t mergeIndex) {
    const size_t k = chunkFiles.size();
    if (k == 0) return "";
    
    // Open all chunk files for reading
    std::vector<std::ifstream> inputs(k);
    for (size_t i = 0; i < k; ++i) {
        inputs[i].open(chunkFiles[i], std::ios::binary);
        if (!inputs[i]) {
            throw std::runtime_error("Could not open chunk file: " + chunkFiles[i]);
        }
    }
    
    // Min-heap using std::priority_queue (use std::greater for min-heap behavior)
    std::priority_queue<HeapNode, std::vector<HeapNode>, std::greater<HeapNode>> minHeap;
    
    // Initialize heap with first element from each chunk
    for (size_t i = 0; i < k; ++i) {
        double value;
        if (inputs[i].read(reinterpret_cast<char*>(&value), sizeof(double))) {
            minHeap.push({value, i});
        }
    }
    
    // Open output file
    std::string outputFile = "merged_" + std::to_string(mergeIndex) + ".bin";
    std::ofstream out(outputFile, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Could not open output file: " + outputFile);
    }
    
    // K-way merge using the min-heap
    while (!minHeap.empty()) {
        // Extract minimum element
        HeapNode minNode = minHeap.top();
        minHeap.pop();
        
        // Write to output
        out.write(reinterpret_cast<const char*>(&minNode.value), sizeof(double));
        
        // Read next element from the same chunk and add to heap
        double nextValue;
        if (inputs[minNode.chunkIndex].read(reinterpret_cast<char*>(&nextValue), sizeof(double))) {
            minHeap.push({nextValue, minNode.chunkIndex});
        }
    }
    
    // Close all files
    out.close();
    for (size_t i = 0; i < k; ++i) {
        inputs[i].close();
    }
    
    // Clean up temporary chunk files
    for (const auto& chunkFile : chunkFiles) {
        std::remove(chunkFile.c_str());
    }
    
    return outputFile;
}

void ExternalSorter::mergeSortedChunks(const std::vector<std::string>& chunkFiles, const std::string& outputFile) {
    if (chunkFiles.empty()) return;
    
    std::vector<std::string> currentChunks = chunkFiles;
    size_t mergePass = 0;
    
    // Multi-pass merge: merge MAX_MERGE_FILES at a time until only one remains
    while (currentChunks.size() > MAX_MERGE_FILES) {
        std::vector<std::string> nextChunks;
        
        for (size_t i = 0; i < currentChunks.size(); i += MAX_MERGE_FILES) {
            size_t end = std::min(i + MAX_MERGE_FILES, currentChunks.size());
            std::vector<std::string> subset(currentChunks.begin() + i, currentChunks.begin() + end);
            std::string merged = mergeSubset(subset, mergePass * 1000 + i);
            nextChunks.push_back(merged);
        }
        
        currentChunks = nextChunks;
        mergePass++;
    }
    
    // Final merge to the output file
    if (currentChunks.size() == 1) {
        // Just rename the last chunk to output file
        std::ifstream src(currentChunks[0], std::ios::binary);
        std::ofstream dst(outputFile, std::ios::binary);
        dst << src.rdbuf();
        src.close();
        dst.close();
        std::remove(currentChunks[0].c_str());
    } else {
        // Merge remaining chunks directly to output
        const size_t k = currentChunks.size();
        std::vector<std::ifstream> inputs(k);
        for (size_t i = 0; i < k; ++i) {
            inputs[i].open(currentChunks[i], std::ios::binary);
            if (!inputs[i]) {
                throw std::runtime_error("Could not open chunk file: " + currentChunks[i]);
            }
        }
        
        std::priority_queue<HeapNode, std::vector<HeapNode>, std::greater<HeapNode>> minHeap;
        
        for (size_t i = 0; i < k; ++i) {
            double value;
            if (inputs[i].read(reinterpret_cast<char*>(&value), sizeof(double))) {
                minHeap.push({value, i});
            }
        }
        
        std::ofstream out(outputFile, std::ios::binary);
        if (!out) {
            throw std::runtime_error("Could not open output file: " + outputFile);
        }
        
        while (!minHeap.empty()) {
            HeapNode minNode = minHeap.top();
            minHeap.pop();
            out.write(reinterpret_cast<const char*>(&minNode.value), sizeof(double));
            
            double nextValue;
            if (inputs[minNode.chunkIndex].read(reinterpret_cast<char*>(&nextValue), sizeof(double))) {
                minHeap.push({nextValue, minNode.chunkIndex});
            }
        }
        
        out.close();
        for (size_t i = 0; i < k; ++i) {
            inputs[i].close();
        }
        
        for (const auto& chunkFile : currentChunks) {
            std::remove(chunkFile.c_str());
        }
    }
}

void ExternalSorter::sort(const std::string& inputFile, const std::string& outputFile) {
    // Phase 1: Split input into sorted chunks
    std::vector<std::string> chunkFiles = SplitandSort(inputFile);
    
    // Phase 2: Merge all sorted chunks using heap-based k-way merge
    mergeSortedChunks(chunkFiles, outputFile);
}
