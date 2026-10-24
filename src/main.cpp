#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include "pipelines/bzip2_pipeline.hpp"
#include "pipelines/lzma_pipeline.hpp"
#include "pipelines/7zip_pipeline.hpp"
#include "utils/file_io.hpp"

using namespace algozip;

// Serialise un entier 32 bits en 4 octets big-endian
static void write_int(std::vector<uint8_t>& buf, int value) {
    buf.push_back((value >> 24) & 0xFF);
    buf.push_back((value >> 16) & 0xFF);
    buf.push_back((value >>  8) & 0xFF);
    buf.push_back((value >>  0) & 0xFF);
}

// Lit un entier 32 bits depuis 4 octets big-endian
static int read_int(const std::vector<uint8_t>& buf, size_t& pos) {
    if (pos + 4 > buf.size()) {
        throw std::runtime_error("Buffer too short");
    }
    int value = ((int)buf[pos]     << 24) |
                ((int)buf[pos + 1] << 16) |
                ((int)buf[pos + 2] <<  8) |
                ((int)buf[pos + 3] <<  0);
    pos += 4;
    return value;
}

static void print_usage() {
    std::cout << "Usage:\n"
              << "  algozip compress   --mode [bzip2|lzma|7zip] <input> <output>\n"
              << "  algozip decompress --mode [bzip2|lzma|7zip] <input> <output>\n"
              << "\nExamples:\n"
              << "  algozip compress   --mode bzip2 file.txt file.az\n"
              << "  algozip decompress --mode bzip2 file.az  file.txt\n"
              << "  algozip compress   --mode lzma  file.txt file.az\n"
              << "  algozip decompress --mode lzma  file.az  file.txt\n"
              << "  algozip compress   --mode 7zip  file.txt file.az\n"
              << "  algozip decompress --mode 7zip  file.az  file.txt\n";
}

// Compresse un fichier par blocs
// Format du fichier compresse :
// [4 bytes] nombre de blocs
// Pour chaque bloc :
//   [4 bytes] taille du bloc compresse
//   [N bytes] contenu du bloc compresse
static void compress_file(
    const std::string& input_path,
    const std::string& output_path,
    const std::string& mode
) {
    std::vector<std::vector<uint8_t>> compressed_blocks;
    size_t total_original = 0;

    read_file_blocks(
        input_path,
        BLOCK_SIZE,
        [&](const std::string& block, bool /*is_last*/) {
            total_original += block.size();

            std::vector<uint8_t> compressed_block;
            if (mode == "bzip2") {
                compressed_block = bzip2_compress(block);
            } else if (mode == "lzma") {
                compressed_block = lzma_compress(block);
            } else {   // mode == "7zip"
                compressed_block = sevenzip_compress(block);
            }

            compressed_blocks.push_back(compressed_block);

            std::cout << "  Block " << compressed_blocks.size()
                      << " : " << block.size()
                      << " -> " << compressed_block.size()
                      << " bytes\n";
        }
    );

    std::ofstream out(output_path, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot write: " + output_path);
    }

    std::vector<uint8_t> header;
    write_int(header, static_cast<int>(compressed_blocks.size()));
    out.write(reinterpret_cast<const char*>(header.data()), header.size());

    size_t total_compressed = 4;
    for (const auto& block : compressed_blocks) {
        std::vector<uint8_t> block_header;
        write_int(block_header, static_cast<int>(block.size()));
        out.write(reinterpret_cast<const char*>(block_header.data()),
                  block_header.size());
        out.write(reinterpret_cast<const char*>(block.data()),
                  block.size());
        total_compressed += 4 + block.size();
    }

    std::cout << "Original size    : " << total_original << " bytes\n";
    std::cout << "Compressed size  : " << total_compressed << " bytes\n";
    double ratio = total_original == 0
        ? 0.0
        : 100.0 * static_cast<double>(total_compressed) / total_original;
    std::cout << "Compression ratio: " << ratio << "%\n";
    std::cout << "Blocks           : " << compressed_blocks.size() << "\n";
    std::cout << "Output written to: " << output_path << "\n";
}

// Decompresse un fichier par blocs
static void decompress_file(
    const std::string& input_path,
    const std::string& output_path,
    const std::string& mode
) {
    std::vector<uint8_t> compressed = read_file_bytes(input_path);

    size_t pos = 0;
    int num_blocks = read_int(compressed, pos);

    std::cout << "Blocks to decompress: " << num_blocks << "\n";

    std::ofstream out(output_path, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot write: " + output_path);
    }

    size_t total_decompressed = 0;

    for (int i = 0; i < num_blocks; i++) {
        int block_size = read_int(compressed, pos);

        if (pos + block_size > compressed.size()) {
            throw std::runtime_error("Truncated block in input file");
        }

        std::vector<uint8_t> block(
            compressed.begin() + pos,
            compressed.begin() + pos + block_size
        );
        pos += block_size;

        std::string decompressed_block;
        if (mode == "bzip2") {
            decompressed_block = bzip2_decompress(block);
        } else if (mode == "lzma") {
            decompressed_block = lzma_decompress(block);
        } else {   // mode == "7zip"
            decompressed_block = sevenzip_decompress(block);
        }

        out.write(decompressed_block.data(), decompressed_block.size());
        total_decompressed += decompressed_block.size();

        std::cout << "  Block " << (i + 1)
                  << " : " << block_size
                  << " -> " << decompressed_block.size()
                  << " bytes\n";
    }

    std::cout << "Decompressed size: " << total_decompressed << " bytes\n";
    std::cout << "Output written to: " << output_path << "\n";
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        print_usage();
        return 1;
    }

    std::string command     = argv[1];
    std::string mode_flag   = argv[2];
    std::string mode        = argv[3];
    std::string input_path  = argv[4];
    std::string output_path = argv[5];

    if (mode_flag != "--mode") {
        std::cerr << "Error: expected --mode flag\n";
        print_usage();
        return 1;
    }

    if (mode != "bzip2" && mode != "lzma" && mode != "7zip") {
        std::cerr << "Error: mode must be 'bzip2', 'lzma' or '7zip'\n";
        print_usage();
        return 1;
    }

    if (command != "compress" && command != "decompress") {
        std::cerr << "Error: command must be 'compress' or 'decompress'\n";
        print_usage();
        return 1;
    }

    try {
        if (command == "compress") {
            std::cout << "Compressing '" << input_path
                      << "' with " << mode << "...\n";
            compress_file(input_path, output_path, mode);
        } else {
            std::cout << "Decompressing '" << input_path
                      << "' with " << mode << "...\n";
            decompress_file(input_path, output_path, mode);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
