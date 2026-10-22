#include "utils/file_io.hpp"
#include <fstream>
#include <stdexcept>

namespace algozip {

void read_file_blocks(
    const std::string& path,
    size_t block_size,
    std::function<void(const std::string&, bool)> callback
) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::string buffer(block_size, '\0');

    while (true) {
        file.read(&buffer[0], block_size);
        std::streamsize bytes_read = file.gcount();

        if (bytes_read == 0) break;

        bool is_last = file.eof();
        callback(buffer.substr(0, bytes_read), is_last);

        if (is_last) break;
    }
}

void write_file_bytes(
    const std::string& path,
    const std::vector<uint8_t>& data
) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write file: " + path);
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

std::vector<uint8_t> read_file_bytes(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    return std::vector<uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

void write_file_string(
    const std::string& path,
    const std::string& data
) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write file: " + path);
    }
    file.write(data.data(), data.size());
}

std::string read_file_string(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

} // namespace algozip
