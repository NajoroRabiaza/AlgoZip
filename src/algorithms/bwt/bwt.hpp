#ifndef BWT_HPP
#define BWT_HPP
#include <string>
#include <vector>

namespace algozip {
    // resultat de l'encodage BWT
    // transformed : la chaine transformer (ex: "NNBAAA")
    // index : position de la chaine originale dans le tableau trier
    struct BWTResult {
        std::string transformed;
        int index;
    };

    // encode une chaine avec la Burrows-wheeler transform
    // input : la chaine a transformer (ex: "BANANA")
    // retourne un BWTResult contenant la chaine transformer et l'index
    BWTResult bwt_encode(const std::string& input);

    // decode une chaine transformer par BWT
    // transformed : la chaine issue de bwt_encoder (ex: "NNBAAA")
    // index : l'index retourner par bwt_encode (ex: 4)
    // retourne la chaine originale (ex: "BANANA")
    std::string bwt_decode(const std::string & transformed, int index);
}
#endif