#ifndef LZMA_PIPELINE_HPP
#define LZMA_PIPELINE_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace algozip {
    // compresse une chaine avec le pipeline lzma
    // etape 1 : LZ77 encode => produit les triplets
    // etape 2 : Arithmetic encode => compresse la representation des triplets
    // retourne un bloc binaire autoportant
    std::vector<uint8_t> lzma_compress(const std::string& input);

    // decompresse un bloc produit par lzma_compress
    // etape 1 : arithmetic decode => retrouve la representation des triplets
    // etape 2 : LZ77 decode => retrouve la chaine originale
    std::string lzma_decompress(const std::vector<uint8_t>& compressed);
}

#endif