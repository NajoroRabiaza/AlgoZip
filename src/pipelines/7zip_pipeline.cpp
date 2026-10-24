#include "7zip_pipeline.hpp"
#include "algorithms/lz77/lz77.hpp"
#include "algorithms/huffman/huffman.hpp"
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <map>

namespace algozip {

static void write_int(std::vector<uint8_t>& buf, int value) {
    buf.push_back((value >> 24) & 0xFF);
    buf.push_back((value >> 16) & 0xFF);
    buf.push_back((value >>  8) & 0xFF);
    buf.push_back((value >>  0) & 0xFF);
}

static int read_int(const std::vector<uint8_t>& buf, size_t& pos) {
    if (pos + 4 > buf.size()) {
        throw std::runtime_error("Buffer too short to read int");
    }
    int value = ((int)buf[pos]     << 24) |
                ((int)buf[pos + 1] << 16) |
                ((int)buf[pos + 2] <<  8) |
                ((int)buf[pos + 3] <<  0);
    pos += 4;
    return value;
}

// Convertit les tokens LZ77 en vecteur d octets
// Chaque token = 5 octets :
// [offset_hi][offset_lo][length][next_char][has_next]
static std::vector<uint8_t> tokens_to_bytes(
        const std::vector<LZ77Token>& tokens) {
    std::vector<uint8_t> result;
    for (const LZ77Token& t : tokens) {
        result.push_back((t.offset >> 8) & 0xFF);
        result.push_back(t.offset & 0xFF);
        result.push_back(t.length & 0xFF);
        result.push_back(t.next == '\0' ? 0x00 : (uint8_t)t.next);
        result.push_back(t.next == '\0' ? 0x00 : 0x01);
    }
    return result;
}

// Reconstruit les tokens depuis les octets
static std::vector<LZ77Token> bytes_to_tokens(
        const std::vector<uint8_t>& bytes, int num_tokens) {
    std::vector<LZ77Token> tokens;
    for (int i = 0; i < num_tokens; i++) {
        size_t pos = i * 5;
        if (pos + 5 > bytes.size()) break;
        int  offset   = ((int)bytes[pos] << 8) | (int)bytes[pos + 1];
        int  length   = (int)bytes[pos + 2];
        bool has_next = bytes[pos + 4] == 0x01;
        char next     = has_next ? (char)bytes[pos + 3] : '\0';
        tokens.push_back({offset, length, next});
    }
    return tokens;
}

// Format du bloc compresse 7zip :
// [4 bytes] nombre de tokens LZ77
// [4 bytes] nombre de symboles Huffman distincts
// Pour chaque symbole :
//   [1 byte]  le caractere
//   [4 bytes] sa frequence
// [4 bytes] taille des bits Huffman en bits
// [N bytes] les bits Huffman packs en octets

std::vector<uint8_t> sevenzip_compress(const std::string& input) {
    if (input.empty()) {
        throw std::invalid_argument("Input cannot be empty");
    }

    // Etape 1 : LZ77 encode
    LZ77Result lz77 = lz77_encode(input, 6, 6);

    // Etape 2 : Convertir les tokens en octets
    std::vector<uint8_t> tokens_bytes = tokens_to_bytes(lz77.tokens);

    // Etape 3 : Convertir en string pour Huffman
    // On utilise les octets directement comme symboles
    std::string tokens_str(tokens_bytes.begin(), tokens_bytes.end());

    // Etape 4 : Huffman encode
    HuffmanResult huff = huffman_encode(tokens_str);

    // Construire le bloc compresse
    std::vector<uint8_t> compressed;

    // Nombre de tokens LZ77
    write_int(compressed, static_cast<int>(lz77.tokens.size()));

    // Table des frequences Huffman
    write_int(compressed, static_cast<int>(huff.frequencies.size()));
    for (const auto& pair : huff.frequencies) {
        compressed.push_back(static_cast<uint8_t>(pair.first));
        write_int(compressed, pair.second);
    }

    // Taille des bits Huffman en nombre de bits
    write_int(compressed, static_cast<int>(huff.encoded.size()));

    // Bits Huffman packs en octets
    size_t num_bytes = (huff.encoded.size() + 7) / 8;
    for (size_t i = 0; i < num_bytes; i++) {
        uint8_t byte = 0;
        for (int b = 0; b < 8; b++) {
            size_t bit_pos = i * 8 + b;
            if (bit_pos < huff.encoded.size()) {
                if (huff.encoded[bit_pos] == '1') {
                    byte |= (1 << (7 - b));
                }
            }
        }
        compressed.push_back(byte);
    }

    return compressed;
}

std::string sevenzip_decompress(const std::vector<uint8_t>& compressed) {
    if (compressed.empty()) {
        throw std::invalid_argument("Compressed data cannot be empty");
    }

    size_t pos = 0;

    // Lire le nombre de tokens
    int num_tokens = read_int(compressed, pos);

    // Lire la table des frequences
    int num_symbols = read_int(compressed, pos);
    std::map<char, int> frequencies;
    for (int i = 0; i < num_symbols; i++) {
        char c    = static_cast<char>(compressed[pos++]);
        int  freq = read_int(compressed, pos);
        frequencies[c] = freq;
    }

    // Reconstruire le codebook Huffman depuis les frequences
    std::string ref_str;
    for (const auto& pair : frequencies) {
        for (int i = 0; i < pair.second; i++) {
            ref_str += pair.first;
        }
    }
    HuffmanResult huff_ref = huffman_encode(ref_str);

    // Lire le nombre de bits
    int num_bits = read_int(compressed, pos);

    // Depacker les bits
    std::string encoded_bits;
    encoded_bits.reserve(num_bits);
    size_t num_bytes = (num_bits + 7) / 8;

    for (size_t i = 0; i < num_bytes; i++) {
        uint8_t byte = compressed[pos++];
        for (int b = 0; b < 8; b++) {
            if (static_cast<int>(encoded_bits.size()) < num_bits) {
                encoded_bits += ((byte >> (7 - b)) & 1) ? '1' : '0';
            }
        }
    }

    // Etape 1 : Huffman decode --> retrouve les octets des tokens
    std::string tokens_str = huffman_decode(encoded_bits, huff_ref.codebook);

    // Etape 2 : Convertir la string en octets
    std::vector<uint8_t> tokens_bytes(tokens_str.begin(), tokens_str.end());

    // Etape 3 : Reconstruire les tokens LZ77
    std::vector<LZ77Token> tokens = bytes_to_tokens(tokens_bytes, num_tokens);

    if (static_cast<int>(tokens.size()) != num_tokens) {
        throw std::runtime_error("Token count mismatch");
    }

    // Etape 4 : LZ77 decode
    return lz77_decode(tokens);
}

} // namespace algozip
