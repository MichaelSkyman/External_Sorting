#pragma once
#include <string>

/** @brief Determines how the sort chunk size is calculated. */
enum class ChunkMode {
    AUTO,   ///< Automatically derived from available system RAM.
    MANUAL  ///< Explicitly set by the user via setManualChunkSize().
};

/**
 * @brief Stores application-level configuration for the current sorting session.
 *
 * Manages input/output paths, chunk mode, and chunk size. All values are
 * held in memory only; no persistence to disk is performed.
 */
class AppConfig {
public:
    /// @brief Sets the path to the binary input file.
    /// @param path Absolute or relative path to the input .bin file.
    void setInputFile(const std::string& path);

    /// @brief Sets the output directory where output.bin will be written.
    /// @param path Absolute or relative path to the output directory.
    void setOutputDir(const std::string& path);

    /// @brief Sets the chunk size mode (AUTO or MANUAL).
    /// @param mode The desired ChunkMode.
    void setChunkMode(ChunkMode mode);

    /// @brief Sets the manual chunk size used when mode is MANUAL.
    /// @param size Number of double elements per in-memory chunk.
    void setManualChunkSize(size_t size);

    /// @brief Returns the configured input file path.
    std::string getInputFile() const;

    /// @brief Returns the configured output directory path.
    std::string getOutputDir() const;

    /// @brief Returns the effective chunk size.
    ///
    /// In AUTO mode, derives the chunk size from available RAM.
    /// In MANUAL mode, returns the value set via setManualChunkSize().
    size_t getChunkSize() const;

private:
    std::string inputFile;                   ///< Path to the binary input file.
    std::string outputDir;                   ///< Path to the output directory.
    ChunkMode   chunkMode   = ChunkMode::AUTO; ///< Active chunk size mode.
    size_t      manualChunkSize = 0;           ///< User-specified chunk size (MANUAL mode only).
};
