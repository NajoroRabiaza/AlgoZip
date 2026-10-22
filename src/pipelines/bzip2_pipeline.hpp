#ifndef BZIP2_PIPELINE_HPP
#define BZIP2_PIPELINE_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace algozip {
    // compresse une chaine avec le pipeline bzip2
    // etape 1 : BWT encode => transforme la chaine
    // etape 2 : Huffman encode => compresse la chaine transformer
    // retourne un bloc binaire autoportant contenant tout
    // ce qui est necessaire pour decompresser
    std::vector<uint8_t> bzip2_compress(const std::string& input);

    // decompresse un bloc produit par bzip2_compress
    // etape 1 : Huffman decode => retrouve la chaine transformer
    // etape 2 : BWT decode => retrouve la chaine originale
    std::string bzip2_decompress(const std::vector<uint8_t>& compressed);
}

#endif