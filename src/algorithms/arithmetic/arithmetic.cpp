#include "arithmetic.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace algozip {
    // construit le modele de proba depuis la chaine d'entrer
    // correspond exactement aux etapes du cours :
    // 1 - compter les occurences de chaque symbole (ni)
    // 2 - calculer P(si) = ni / n
    // 3 - calculer les intervalles cumuler C(si-1) et C(si)
    static std::vector<SymbolModel> build_model(const std::string& input) {
        int n = input.size();

        // compter les occurences dans l'ordre d'apparition
        // on utilise un vecteur pour preserver l'ordre du cours
        std::map<char, int> counts;
        std::vector<char> order;

        for (char c : input) {
            if (counts.find(c) == counts.end()) {
                order.push_back(c);
            }
            counts[c]++;
        }

        // construire le modele dans l'ordre d'apparition
        std::vector<SymbolModel> model;
        double cumulative = 0.0;

        for (char c : order) {
            SymbolModel sym;
            sym.symbol = c;
            sym.count = counts[c];
            sym.probability = static_cast<double>(counts[c]) / static_cast<double>(n);
            sym.low = cumulative;
            sym.high = cumulative + sym.probability;
            cumulative = sym.high;
            model.push_back(sym);
        }

        for (const SymbolModel& s : model) {
        }

        // s'assurer que la borne haute du dernier symbole est exactement 1.0
        // pour eviter les erreurs d'arrondi flottant
        if (!model.empty()) {
            model.back().high = 1.0;
        }
        return model;
    }

    // cherche le symbole dont l'intervalle [low, high[ contient la valeur du code
    // correspond a l'etape 1 du decodage du cours :
    // "trouver si tel que C(si-1) <= D < C(si)"
    static const SymbolModel* find_symbol(const std::vector<SymbolModel>& model, double code) {
        for (const SymbolModel& sym : model) {
            if (code >= sym.low && code < sym.high) {
                return &sym;
            }
        }
        // cas limite : code tres proche de 1.0 (erreurs d'arrondi)
        // on retourne le dernier symbole
        return &model.back();
    }
    
    ArithmeticResult arithmetic_encode(const std::string& input) {
        if (input.empty()) {
            throw std::invalid_argument("Input string cannot be empty");
        }

        int n = input.size();
        std::vector<SymbolModel> model = build_model(input);

        // encoder chaque symbole en affinant l'intervalle [L, R]
        // les formules :
        // Lj = Lj-1 + (Rj-1 - Lj-1) x C(si-1)
        // Rj = Lj-1 + (Rj-1 - Lj-1) x C(si)
        double L = 0.0;
        double R = 1.0;

        for (char c : input) {
            // trouver le symbole courant dans le modele
            const SymbolModel* sym = nullptr;
            for (const SymbolModel& s : model) {
                if (s.symbol == c) {
                    sym = &s;
                    break;
                }
            }
            if (sym == nullptr) {
                throw std::runtime_error("symbol not found in model");
            }

            // affiner l'intervalle
            double range = R - L;
            R = L + range * sym->high;
            L = L + range * sym->low;

            for (const SymbolModel& s : model) {
            }
        }

        // le code final est le milieu de l'intervalle final [L, R]
        // conforme dans le cours avec (Ln + Rn) / 2
        double code = (L + R) / 2.0;

        

        return {code, model, n};
    }

    std::string arithmetic_decode(double code, const std::vector<SymbolModel>& model, int n) {
        if (model.empty()) {
            throw std::invalid_argument("model cannot be empty");
        }
        if (n<=0) {
            throw std::invalid_argument("N must be strictly positive");
        }
        if(code < 0.0 || code > 1.0) {
            throw std::invalid_argument("Code must be between 0 and 1");
        }

        std::string decoded = "";
        double D = code;

        // decoder N symboles
        // algorithme dans le cours :
        // 1 : trouver si tel que C(si-1) <= D < C(si)
        // 2 : emettre si
        // 3 : mettre a jour D = (D - C(si-1)) / P(si)
        // repeter N fois
        for (int j = 0; j < n; j++) {
            const SymbolModel* sym = find_symbol(model, D);
            if (sym == nullptr) {
                throw std::runtime_error("cannot find symbol for code "+ std::to_string(D));
            }
            decoded += sym->symbol;

            // Mise a jour de D
            D = (D - sym->low) / sym->probability;
        }
        return decoded;
    }
}