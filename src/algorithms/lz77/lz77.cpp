#include "lz77.hpp"

namespace algozip {
    LZ77Result lz77_encode(const std::string& input, int search_size, int lookahead_size) {
        if (input.empty()) {
            throw std::invalid_argument("Input string cannot be empty");
        }

        LZ77Result result;
        int pos = 0;
        int n = input.size();

        while (pos < n) {
            // definir les bornes du buffer de recherche
            // on ne peut pas remonter avant le debut de la chaine
            int search_start = std::max(0, pos - search_size);
            int search_end = pos;

            // definir les bornes du buffer d'entree
            int lookahead_end = std::min(n, pos + lookahead_size);

            int best_offset = 0;
            int best_length = 0;

            // chercher la plus longue correspondance dans le buffer de recherche
            // on compare chaque position du buffer de recherche avec le debut du buffer d'entrer
            for (int i = search_start; i< search_end; i++) {
                int length = 0;

                // compter combien de caracteres correspondent
                // LZ77 permet le chevauchement : la correspondance peut 
                // depasser la position courante (pos) dans la chaine
                // c'est ce qui permet (3,5,D) dans CABRACADABRARRARRAD
                while(length < lookahead_size && pos + length < n && input[i+length] == input[pos+length]) {
                    length++;
                }

                // garder la plus longue correspondance trouver
                if (length > best_length) {
                    best_length = length;
                    // offset est distance entre la position courante et le debut de la
                    // correpondance dans le buffer de recherche
                    best_offset = pos - i;
                }
            }

            // construire le triplet
            // si on est au dernier caractere et qu'on a une correspondance
            // on emet le triplet sans caracteres suivant
            char next_char = '\0';
            if (pos + best_length < n) {
                next_char = input[pos + best_length];
            }

            result.tokens.push_back({best_offset, best_length, next_char});

            // avancer la chaine
            // on saute la correspondance et le caractere suivant
            pos += best_length + 1;
        }
        return result;
    }

    std::string lz77_decode(const std::vector<LZ77Token>& tokens) {
        if (tokens.empty()) {
            throw std::invalid_argument("Tokens list cannot be empty");
        }

        std::string decoded = "";

        for (const LZ77Token& token : tokens) {
            if (token.offset == 0 && token.length == 0) {
                // pas de correspondance : emission directe du caractere
                if(token.next != '\0') {
                    decoded += token.next;
                }
            } else {
                // corespondance trouver
                // on recule de 'offset' positions dans le texte deja reconstruit
                int start = decoded.size() - token.offset;

                // on copie 'length' caracteres.
                // copie possible caractere par caracteres
                // car la la correspondance peut chevaucher la position courante
                for (int i = 0; i < token.length; i++) {
                    decoded += decoded[start + i];
                }

                // on ajoute le caractere suivant
                if (token.next != '\0') {
                    decoded += token.next;
                }
            }
        }
        return decoded;
    }
}