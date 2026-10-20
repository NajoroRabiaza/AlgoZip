#include "bwt.hpp"
#include <algorithm>
#include <stdexcept>

namespace algozip {
    BWTResult bwt_encode(const std::string& input) {
        if(input.empty()) {
            throw std::invalid_argument("Input string cannot be empty");
        }

        int n = input.size();

        // etape 1 : generer tout les rotations
        // Ex : "BANANA" --> ["BANANA", "ANANAB", "NANABA", ...]
        std::vector<std::string> rotations(n);
        for (int i = 0; i<n; i++) {
            rotations[i] = input.substr(i) + input.substr(0, i);
        }

        // etape 2 : trier les rotations dans l'ordre lexicographique
        // Ex: ["ABANAN", "ANABAN", "ANANAB", "BANANA", "NABANA", "NANABA"]
        std::sort(rotations.begin(), rotations.end());

        // etape 3 : construire la chaine transformer
        // on prend le dernier caractere de chaque rotation trier
        // ex: "NNBAAA"
        std::string transformed(n, ' ');
        int index = -1;
        for (int i = 0; i<n; i++) {
            transformed[i] = rotations[i].back();
            if (rotations[i] == input) {
                index = i;
            }
        }
        return {transformed, index};
    }

    std::string bwt_decode(const std::string& transformed, int index) {
        if(transformed.empty()) {
            throw std::invalid_argument("Transformed string cannot be empty");
        }

        int n = transformed.size();

        // on reconstruit par la methode des colonnes
        // documenter dans BWT.docx (C1 a C12 pour BANANA)
        // a chaque iteration on colle la colonne transformer
        // devant la colonne courante puis on retrie

        std::vector<std::string> table(n);
        // initialiser avec des chaines vides
        for (int i=0; i<n; i++) {
            table[i] = "";
        }

        // iterer n fois : coller + trier
        for (int step = 0; step < n; step++) {
            // coller le caractere transformer devant chaque entrer
            for (int i = 0; i < n; i++) {
                table[i] = transformed[i] + table[i];
            }

            // trier lexicographiquement
            std::sort(table.begin(), table.end());
        }

        // chaine originale a la position index
        return table[index];
    }
}