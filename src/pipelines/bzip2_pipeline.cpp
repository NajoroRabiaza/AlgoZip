#include "bzip2_pipeline.hpp"
#include "algorithms/bwt/bwt.hpp"
#include "algorithms/huffman/huffman.hpp"
#include <stdexcept>
#include <cstring>
#include <cstdint>

namespace algozip {
    // format du bloc compresser bzip2 :
    // [4 bytes] taille originale (int)
    // [4 bytes] index BWT (int)
    // [4 bytes] nombre de symboles distincts (int)
    // Pour chaque symbole :
    //  [1 bytes] le caractere
    //  [4 bytes] la frequence
    // [4 bytes] taille des bits Huffman en bits (int)
    // [N bytes] les bits Huffman packs en octets

    // serialise un entier 32 bits en 4 octets big-endian
    static void write_int(std::vector<uint8_t>& buf, int value) {
        buf.push_back((value >> 24) & 0xFF);
        buf.push_back((value >> 16) & 0xFF);
        buf.push_back((value >> 8) & 0xFF);
        buf.push_back((value >> 0) & 0xFF);
    }

    // lit un entier 32 bits depuis 4 octets big-endian
    static int read_int(const std::vector<uint8_t>& buf, size_t& pos) {
        if (pos + 4 > buf.size()) {
            throw std::runtime_error("Buffer too short to read int");
        }
        int value = (buf[pos]   << 24) |
                    (buf[pos + 1] << 16) |
                    (buf[pos + 2] << 8) |
                    (buf[pos + 3] << 0);
        pos += 4;
        return value;
    }

    std::vector<u_int8_t> bzip2_compress(const std::string& input) {
        if (input.empty()) {
            throw std::invalid_argument("Input cannot be empty");
        }

        // etape 1 : BWT encode
        // transforme la chaine pour regrouper les caracteres similaires
        // ex : "BANANA" => ("NNBAAA", index=3)
        BWTResult bwt = bwt_encode(input);

        // etape 2 : Huffman encode
        // compresse la chaine transformer par BWT
        // les caracteres similaires regrouper par BWT sont plus
        // efficace a comprimer par huffman
        HuffmanResult huff = huffman_encode(bwt.transformed);

        // construire le bloc compresse
        std::vector<uint8_t> compressed;

        // en-tete : taille originale
        write_int(compressed, static_cast<int>(input.size()));

        // en-tete : index BWT
        write_int(compressed, bwt.index);

        // en-tete : nombre de symboles distincts dans le codebook
        write_int(compressed, static_cast<int>(huff.frequencies.size()));

        // en-tete : table des frequences
        // necessaire pour reconstruire le codebook au decodage
        for (const auto& pair : huff.frequencies) {
            compressed.push_back(static_cast<uint8_t>(pair.first));
            write_int(compressed, pair.second);
        }

        // en-tete : taille des bits Huffman en nombre de bits
        write_int(compressed, static_cast<int>(huff.encoded.size()));

        // payload : les bits Huffman packs en octets (8 bits par octet)
        // on ecite de stocker un char '0' ou '1' par bit
        // qui prendrait 8x plus de place
        size_t num_bytes = (huff.encoded.size() + 7) / 8;
        for (size_t i = 0; i<num_bytes; i++) {
            uint8_t byte = 0;
            for (int b = 0; b<8; b++) {
                size_t bit_pos = i * 8 + b;
                if (bit_pos < huff.encoded.size()) {
                    if (huff.encoded[bit_pos] == '1') {
                        byte |= (1 << (7-b));
                    }
                }
            }
            compressed.push_back(byte);
        }
        return compressed;
    }

    std::string bzip2_decompress(const std::vector<uint8_t>& compressed) {
        if (compressed.empty()) {
            throw std::invalid_argument("Compressed data cannot be empty");
        }

        size_t pos = 0;

        // lire l'en tete
        int original_size = read_int(compressed, pos);
        int bwt_index = read_int(compressed, pos);
        int num_symbols = read_int(compressed, pos);

        // reconstruire la table des frequences
        std::map<char, int> frequencies;
        for (int i = 0; i<num_symbols; i++) {
            char c = static_cast<char>(compressed[pos++]);
            int freq = read_int(compressed, pos);
            frequencies[c] = freq;
        }

        // reconstruire le codebook depuis les frequences
        // on reconstruit l'arbre de Huffman exactement comme a l'encodage
        HuffmanResult huff_ref = huffman_encode(
            [&]() {
                std::string s;
                for(const auto& pair : frequencies) {
                    for (int i = 0; i<pair.second; i++) {
                        s += pair.first;
                    }
                }
                return s;
            }()
        );

        // lire le nombre de bits Huffman
        int num_bits = read_int(compressed, pos);

        // depacker les bits depuis les octets
        std::string encoded_bits;
        encoded_bits.reserve(num_bits);
        size_t num_bytes = (num_bits + 7) / 8;

        for (size_t i = 0; i< num_bytes; i++) {
            uint8_t byte = compressed[pos++];
            for (int b=0; b<8; b++) {
                if (static_cast<int>(encoded_bits.size() < num_bits)) {
                    encoded_bits += ((byte >> (7 - b)) & 1) ? '1' : '0';
                }
            }
        }

        // etape 1 : Huffmane decode
        std::string bwt_transformed = huffman_decode(encoded_bits, huff_ref.codebook);
        // etape 2 : BWT decode
        std::string original = bwt_decode(bwt_transformed, bwt_index);

        if (static_cast<int>(original.size()) != original_size) {
            throw std::runtime_error("Decompressed size mismatch");
        }
        return original;
    }
}