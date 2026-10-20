#include "lz77.hpp"
#include <algorithm>
#include <stdexcept>

namespace algozip {

LZ77Result lz77_encode(const std::string& input, int search_size, int lookahead_size) {
    if (input.empty()) {
        throw std::invalid_argument("Input string cannot be empty");
    }
    if (search_size <= 0 || lookahead_size <= 0) {
        throw std::invalid_argument("Window sizes must be strictly positive");
    }

    LZ77Result result;
    const int n = static_cast<int>(input.size());
    int pos = 0;

    while (pos < n) {
        const int search_start = std::max(0, pos - search_size);
        const int lookahead_end = std::min(n, pos + lookahead_size);
        const int max_len = lookahead_end - pos;

        int best_offset = 0;
        int best_length = 0;

        // Parcours du plus PROCHE au plus éloigné => en cas d'égalité
        // de longueur, on garde le plus petit offset (moins coûteux en bits).
        // C'est la convention qui fait apparaître (2,1,D) et non (5,1,D)
        // dans l'exemple CABRACADABRARRARRAD du cours.
        for (int i = pos - 1; i >= search_start; --i) {
            if (best_length == max_len) break;  // déjà optimal
            int len = 0;
            // Overlap autorisé : input[i+len] peut dépasser pos.
            while (len < max_len && input[i + len] == input[pos + len]) {
                ++len;
            }
            if (len > best_length) {
                best_length = len;
                best_offset = pos - i;
            }
        }

        if (pos + best_length < n) {
            result.tokens.push_back({best_offset, best_length,
                                     input[pos + best_length]});
            pos += best_length + 1;
        } else {
            // Le match va jusqu'à la fin : pas de "next".
            result.tokens.push_back({best_offset, best_length, '\0'});
            pos += best_length;
        }
    }
    return result;
}

std::string lz77_decode(const std::vector<LZ77Token>& tokens) {
    if (tokens.empty()) {
        throw std::invalid_argument("Tokens list cannot be empty");
    }

    std::string decoded;
    for (const LZ77Token& t : tokens) {
        if (t.length == 0) {
            // Émission littérale (offset doit être 0 par construction)
            if (t.offset != 0) {
                throw std::invalid_argument("Non-zero offset with zero length");
            }
            if (t.next != '\0') decoded += t.next;
            continue;
        }
        if (t.offset <= 0 || t.offset > static_cast<int>(decoded.size())) {
            throw std::invalid_argument("Invalid offset in token");
        }
        const int start = static_cast<int>(decoded.size()) - t.offset;
        for (int j = 0; j < t.length; ++j) {
            decoded += decoded[start + j];   // overlap géré naturellement
        }
        if (t.next != '\0') decoded += t.next;
    }
    return decoded;
}

}