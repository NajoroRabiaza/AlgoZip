#ifndef LZ77_HPP
#define LZ77_HPP
#include <string>
#include <vector>
#include <stdexcept>

namespace algozip {
    // un triplet LZ77 : (offset, longueur, caracter_suivnt)
    // offset c'est disctance en arriere dans le buffer de recherche
    // longueur c'est le nombre de caracters a copier
    // next est le caracter qui suit immediatement la correspondance
    // si pas de correspondance : offset=0, longueur=0, next=caractere courant
    struct LZ77Token {
        int offset;
        int length;
        char next;
    };

    // resutat de l'encodage LZ77
    struct LZ77Result {
        std::vector<LZ77Token> tokens;
    };

    // encode une chaine avec LZ77
    // input : la chaine a encoder
    // search_size : la taille du buffer de recherche (defaut 7)
    // lookahead_size : la taille du buffer d'entree (defaut 6)
    LZ77Result lz77_encode(const std::string& input, int search_size = 7, int lookahead_size = 6);

    // decode une liste de triplets LZ77
    // tokens : les triplets produits par lz77_encode
    // retourne la chaine originale
    std::string lz77_decode(const std::vector<LZ77Token>& tokens);
}

#endif