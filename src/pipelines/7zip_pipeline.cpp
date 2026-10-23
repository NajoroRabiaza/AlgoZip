#include "7zip_pipeline.hpp"
#include "algorithms/lz77/lz77.hpp"
#include "algorithms/huffman/huffman.hpp"
#include <stdexcept>
#include <cstdint>
#include <cstring>

namespace algozip {

// Serialisation identique aux autres pipelines
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

// Convertit les tokens LZ77 en chaine de caracteres
// pour pouvoir appliquer Huffman dessus
// On encode chaque token comme :
// offset(2 bytes) + length(1 byte) + next(1 byte)
// Ce qui donne une chaine de caracteres compressable par Huffman
static std::string tokens_to_string(const std::vector<LZ77Token>& tokens) {
    std::string result;
    for (const LZ77Token& t : tokens) {
        // Encoder offset sur 2 caracteres
        result += static_cast<char>((t.offset >> 8) & 0xFF);
        result += static_cast<char>(t.offset & 0xFF);
        // Encoder length sur 1 caractere
        result += static_cast<char>(t.length & 0xFF);
        // Encoder next sur 1 caractere (0 si absent)
        result += (t.next == '\0') ? static_cast<char>(0) : t.next;
    }
    return result;
}

// Reconstruit les tokens depuis la chaine
static std::vector<LZ77Token> string_to_tokens(const std::string& s) {
    std::vector<LZ77Token> tokens;
    for (size_t i = 0; i + 3 < s.size(); i += 4) {
        int  offset = ((unsigned char)s[i] << 8) | (unsigned char)s[i + 1];
        int  length = (unsigned char)s[i + 2];
        char next   = s[i + 3];
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
    // Identifie les repetitions et produit des triplets
    LZ77Result lz77 = lz77_encode(input, 6, 6);

    // Etape 2 : Convertir les tokens en chaine de caracteres
    // pour pouvoir appliquer Huffman
    std::string tokens_str = tokens_to_string(lz77.tokens);

    // Etape 3 : Huffman encode
    // Compresse la representation des tokens
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

    // Etape 1 : Huffman decode
    std::string tokens_str = huffman_decode(encoded_bits, huff_ref.codebook);

    // Etape 2 : Reconstruire les tokens LZ77
    std::vector<LZ77Token> tokens = string_to_tokens(tokens_str);

    if (static_cast<int>(tokens.size()) != num_tokens) {
        throw std::runtime_error("Token count mismatch after decompression");
    }

    // Etape 3 : LZ77 decode
    return lz77_decode(tokens);
}

} // namespace algozip
