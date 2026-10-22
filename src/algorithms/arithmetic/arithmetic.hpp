#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP

#include <string>
#include <map>
#include <vector>
#include <stdexcept>

namespace algozip {
    // model de probabilite d'un symbole
    // correspond exactement a la table de cours : 
    // i, si, ni, P(si), C(si-1), C(si)
    struct SymbolModel {
        char symbol;
        int count; // ni : nombre d'apparitions
        double probability; // P(si) : ni / n
        double low; // C(si-1) : borne bas de l'intervalle
        double high; // C(si) : borne haut de l'intervalle
    };

    // resultat de l'encodage arithmetique
    struct ArithmeticResult {
        double code; // le nombre code final
        std::vector<SymbolModel> model; // la table des proba
        int n; // nombre total de symbol
    };

    // encode une chaine avec le codage arithmetique
    // input : la chaine a encode (ex: "CALCUL", "ABRACADABRA")
    // retourne un ArithmeticResult avec le code et le modele
    ArithmeticResult arithmetic_encode(const std::string& input);

    // decode un code arithmetique
    // code : le nombre recu (ex: 0.144)
    // model : la table des proba issue de arithmetic_encode
    // n : le nombre de symboles a decoder
    // retourne la chaine originale
    std::string arithmetic_decode(double code, const std::vector<SymbolModel>& model, int n);
}

#endif