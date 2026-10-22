#include "lzma_pipeline.hpp"
#include "algorithms/lz77/lz77.hpp"
#include "algorithms/arithmetic/arithmetic.hpp"
#include <stdexcept>
#include <cstdint>
#include <cstring>

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

    static void write_double(std::vector<uint8_t>& buf, double value) {
        uint8_t bytes[8];
        memcpy(bytes, &value, 8);
        for (int i = 0; i < 8; i++) {
            buf.push_back(bytes[i]);
        }
    }

    static double read_double(const std::vector<uint8_t>& buf, size_t& pos) {
        if (pos + 8 > buf.size()) {
            throw std::runtime_error("Buffer too short to read double");
        }
        double value;
        memcpy(&value, &buf[pos], 8);
        pos += 8;
        return value;
    }


    // convertit les tokens LZ77 en une chaine de caractere
    // pour pouvoir appliquer le codage arithmetique dessus
    // format : "offset, length,next|offset, lengt, next|..."
    static std::string tokens_to_string(const std::vector<LZ77Token>& tokens) {
        std::string result;
        for (const LZ77Token& t : tokens) {
            result += std::to_string(t.offset) + ",";
            result += std::to_string(t.length) + ",";
            result += (t.next == '\0' ? "_" : std::string(1, t.next));
            result += "|";
        }
        return result;
    }

    std::vector<uint8_t> lzma_compress(const std::string& input) {
        if(input.empty()) {
            throw std::invalid_argument("Input cannot be empty");
        }

        // etape 1 : LZ77 encode
        // identifie les repetitions et produit les triplets
        LZ77Result lz77 = lz77_encode(input, 6, 6);

        // etape 2 : arithmetic encode
        // compresse la representation textuelle des tokens
        std::string tokens_str = tokens_to_string(lz77.tokens);
        ArithmeticResult arith = arithmetic_encode(tokens_str);

        // construire le bloc compresse
        std::vector<uint8_t> compressed;

        // nombres de tokens
        write_int(compressed, static_cast<int>(lz77.tokens.size()));

        // stocker chaque token individuellement
        // pour le decodage sans passer par le codage arithmetique
        // le codage arithmetique sert a la compression du flux
        for (const LZ77Token& t : lz77.tokens) {
            write_int(compressed, t.offset);
            write_int(compressed, t.length);
            compressed.push_back(t.next == '\0' ? 0 : static_cast<uint8_t>(t.next));
            compressed.push_back(t.next == '\0' ? 0 : 1);
        }

        // code arithmetique (pour info et verif)
        write_double(compressed, arith.code);

        // modele arithmetique : nbr de symboles
        write_int(compressed, static_cast<int>(arith.model.size()));

        // modele arith : chaque symbole et son count 
        for (const SymbolModel& sym : arith.model) {
            compressed.push_back(static_cast<uint8_t>(sym.symbol));
            write_int(compressed, sym.count);
        }
        return compressed;
    }
    
    std::string lzma_decompress(const std::vector<uint8_t>& compressed) {
        if (compressed.empty()) {
            throw std::invalid_argument("Compressed data cannot be empty");
        }

        size_t pos = 0;

        // lire le nombre de tokens
        int num_tokens = read_int(compressed, pos);

        // reconstruire les tokens LZ77
        std::vector<LZ77Token> tokens;
        tokens.reserve(num_tokens);

        for (int i = 0; i<num_tokens; i++) {
            int offset = read_int(compressed, pos);
            int length = read_int(compressed, pos);
            uint8_t next_char = compressed[pos++];
            uint8_t has_next = compressed[pos++];
            char next = has_next ? static_cast<char>(next_char) : '\0';
            tokens.push_back({offset, length, next});
        }

        // lire le code arithmetique (stocke pour verification)
        double arith_code = read_double(compressed, pos);
        (void)arith_code; // non utiliser dans le decodage direct

        // LZ77 decode : reconstruire la chaine originale depuis les tokens
        return lz77_decode(tokens);
    }
}