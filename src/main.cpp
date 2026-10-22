#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include "pipelines/bzip2_pipeline.hpp"
#include "pipelines/lzma_pipeline.hpp"

using namespace algozip;

// lire un fichier binaire et retourne son contenu
static std::string read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("cannot optn the file : " + path);
    }
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

// ecrit un vecteur d'octets dans un fichier binaire
static void write_file_bytes(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if(!file.is_open()) {
        throw std::runtime_error("cannot write file: " + path);
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

// lire un fichier binaire et retourne son contenu en vecteur d'octets
static std::vector<uint8_t> read_file_bytes(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if(!file.is_open()) {
        throw std::runtime_error("cannot open file: " + path);
    }
    return std::vector<uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

// ecrit une string dans un fichier
static void write_file(const std::string& path, const std::string& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("cannot write file: " + path);
    }
    file.write(data.data(), data.size());
}

static void print_usage() {
    std::cout << "Usage:\n"
                << "algozip compress    --mode [bzip2|lzma] <input> <output>\n"
                << "algozip decompress    --mode [bzip2|lzma] <input> <output>\n"
                << "\nExamples:\n"
                << "algozip compress    --mode bzip2 file.txt file.az\n"
                << "algozip decompress    --mode bzip2 file.az file.txt\n"
                << "algozip compress    --mode lzma file.txt file.az\n"
                << "algozip decompress    --mode lzma file.az file.txt\n";
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        print_usage();
        return 1;
    }
    std::string command = argv[1]; // compress | decompress
    std::string mode_flag = argv[2]; // --mode
    std::string mode = argv[3]; // bzip2 | lzma
    std::string input_path = argv[4]; // fichier entrer

    // pour compress : argv[4] = input, argv[5] = output
    // on reajuste la lecture des arguments
    if (argc != 6) {
        // correction :on a besoin de 6 arguments
        print_usage();
        return 1;
    }

    // relecture correcte avec 6 arguments
    std::string output_path = argv[5];

    if (mode_flag != "--mode") {
        std::cerr << "Error: expected --mode flag\n";
        print_usage();
        return 1;
    }

    if (mode != "bzip2" && mode != "lzma") {
        std::cerr << "error : mode must be 'bzip2' or 'lzma'\n";
        print_usage();
        return 1;
    }

    if (command != "compress" && command != "decompress") {
        std::cerr << "error : command must be 'compress' or 'decompress'\n";
        print_usage();
        return 1;
    }

    try {
        if (command == "compress") {
            std::string input = read_file(input_path);
            std::cout << "compressing '" << input_path
                    << "' with " << mode << "...\n";
            std::cout << "original size : " << input.size() << " bytes\n";
            std::vector<uint8_t> compressed;

            if (mode == "bzip2") {
                compressed = bzip2_compress(input);
            } else {
                compressed = lzma_compress(input);
            }

            write_file_bytes(output_path, compressed);

            std::cout << "Compressed size : " << compressed.size() << " bytes\n";

            double ratio = 100.0 * compressed.size() / input.size();
            std::cout << "Compression ratio : " << ratio << "%\n";
            std::cout << "output written to '" << output_path << "'\n";
        } else {
            std::vector<uint8_t> compressed = read_file_bytes(input_path);
            
            std::cout << "decompressing '" << input_path
                        << "' with " << mode << "...\n";

            std::string decompressed;

            if (mode == "bzip2") {
                decompressed = bzip2_decompress(compressed);
            } else {
                decompressed = lzma_decompress(compressed);
            }

            write_file(output_path, decompressed);

            std::cout << "decompressed size : " << decompressed.size() << " bytes\n";
            std::cout << "output written to '" << output_path << "'\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}