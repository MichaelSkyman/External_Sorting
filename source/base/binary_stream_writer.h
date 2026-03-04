#pragma once
#include <string>
#include <fstream>
#include <cstdint>
#include <stdexcept>
#include <filesystem>

/**
 * BinaryStreamWriter – streams 32-bit signed integers (little-endian)
 * to a binary output file.  Designed for external-sort output stage.
 *
 * Usage:
 *   auto path = BinaryStreamWriter::resolveOutputPath(dir);
 *   BinaryStreamWriter w(path);
 *   w.write(42);               // single value
 *   w.writeFromDouble(3.14);   // truncates to int32
 *   w.close();
 */
class BinaryStreamWriter {
public:
    explicit BinaryStreamWriter(const std::string& filePath)
        : path_(filePath)
    {
        ofs_.open(filePath, std::ios::binary | std::ios::trunc);
        if (!ofs_)
            throw std::runtime_error("BinaryStreamWriter: cannot open " + filePath);
    }

    ~BinaryStreamWriter() { close(); }

    // Non-copyable
    BinaryStreamWriter(const BinaryStreamWriter&) = delete;
    BinaryStreamWriter& operator=(const BinaryStreamWriter&) = delete;

    // Movable
    BinaryStreamWriter(BinaryStreamWriter&&) = default;
    BinaryStreamWriter& operator=(BinaryStreamWriter&&) = default;

    /// Write a single int32 value (little-endian on LE platforms; static_assert below).
    void write(int32_t value) {
        ofs_.write(reinterpret_cast<const char*>(&value), sizeof(int32_t));
    }

    /// Convert a double to int32 (truncation) and write.
    void writeFromDouble(double value) {
        auto iv = static_cast<int32_t>(value);
        write(iv);
    }

    /// Flush and close the stream.
    void close() {
        if (ofs_.is_open()) {
            ofs_.flush();
            ofs_.close();
        }
    }

    /// Return the file path being written to.
    const std::string& filePath() const { return path_; }

    // ----- static helpers -----

    /**
     * Given a directory, resolve the output file path.
     * Default: dir/output.bin
     * If overwrite == false and the file exists, auto-version:
     *   output_1.bin, output_2.bin, …
     */
    static std::string resolveOutputPath(const std::string& dir, bool overwrite = true) {
        namespace fs = std::filesystem;
        fs::path base = fs::path(dir) / "output.bin";

        if (overwrite)
            return base.string();

        if (!fs::exists(base))
            return base.string();

        // Auto-version
        for (int i = 1; i < 100000; ++i) {
            fs::path versioned = fs::path(dir) / ("output_" + std::to_string(i) + ".bin");
            if (!fs::exists(versioned))
                return versioned.string();
        }
        throw std::runtime_error("BinaryStreamWriter: too many output versions in " + dir);
    }

    // ----- static reader helpers -----

    /**
     * Read all int32 values from a binary file into a vector<int32_t>.
     */
    static std::vector<int32_t> readAll(const std::string& filePath) {
        std::ifstream ifs(filePath, std::ios::binary | std::ios::ate);
        if (!ifs)
            throw std::runtime_error("BinaryStreamWriter::readAll: cannot open " + filePath);

        auto size = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        size_t count = static_cast<size_t>(size) / sizeof(int32_t);
        std::vector<int32_t> data(count);
        ifs.read(reinterpret_cast<char*>(data.data()), count * sizeof(int32_t));
        return data;
    }

    /**
     * Read all int32 values and return them as doubles (for visualization).
     */
    static std::vector<double> readAllAsDouble(const std::string& filePath) {
        auto raw = readAll(filePath);
        std::vector<double> result(raw.size());
        for (size_t i = 0; i < raw.size(); ++i)
            result[i] = static_cast<double>(raw[i]);
        return result;
    }

private:
    std::string path_;
    std::ofstream ofs_;

    // Compile-time check: we write the native int32_t representation directly,
    // which is correct only on little-endian (all x86/x64 & ARM-LE).
    static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__,
                  "BinaryStreamWriter assumes little-endian platform");
};
