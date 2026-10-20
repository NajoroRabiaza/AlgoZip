#include "huffman.hpp"
#include <queue>
#include <stdexcept>
#include <functional>

namespace algozip {
    // comparateur pour la file de priorite
    // on veut toujours extraire le noeud avec la plus petite frequence
    struct NodeComparator {
        bool operator()(HuffmanNode* a, HuffmanNode*b) {
            return a->frequency > b->frequency;
        }
    };

    // calcule de la frequence de chaqeu caractere dans la chaine
    static std::map<char, int> compute_frequencies(const std::string& input) {
        std::map<char, int> frequencies;
        for (char c : input) {
            frequencies[c]++;
        }
        return frequencies;
    }

    // construit l'arbre de Huffman depuis les frequences
    // on utilise une file de priorite (min-heap)
    // a chaque etape on extrait les deux noeuds de plus petite frequence
    // on les combien en noeud parent dont la frequence est la somme des deux 
    // on repete jusqu'a ce qu'il ne reste qu'un seul noeud : la racine
    static HuffmanNode* build_tree(const std::map<char, int>& frequencies) {
        std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, NodeComparator> pq;

        // creer une feuille pour chaque symbole
        for (const auto& pair : frequencies) {
            pq.push(new HuffmanNode(pair.first, pair.second));
        }

        // cas special : un seul symbole unique dans la chaine
        // Ex : "AAAA" => un seul noeud, on lui attribue le code "0"
        if (pq.size() == 1) {
            HuffmanNode* only = pq.top(); pq.pop();
            HuffmanNode* root = new HuffmanNode(only->frequency, only, nullptr);
            return root;
        }

        // construire l'arbre en combinant les deux noeuds les plus petits
        while(pq.size() > 1) {
            HuffmanNode* left = pq.top(); pq.pop();
            HuffmanNode* right = pq.top(); pq.pop();
            int combined = left->frequency + right->frequency;
            HuffmanNode* parent = new HuffmanNode(combined, left, right);
            pq.push(parent);
        }
        return pq.top();
    }

    // parcourt l'arbre recursivement pour construire la table des codes
    // branches gauche = "0", branche droite = "1"
    // conforme au cours PDF : 0 pour gauche et 1 pour droite
    static void build_codebook(HuffmanNode* node, const std::string& code, std::map<char, std::string>& codebook) {
        if (node == nullptr) return;

        // si c'est une feuille on enregistre le code
        if (node->left == nullptr && node->right == nullptr) {
            codebook[node->symbol] = code.empty() ? "0" : code;
            return;
        }

        build_codebook(node->left, code + "0", codebook);
        build_codebook(node->right, code + "1", codebook);
    }

    // libre recursivement la memoire de l'arbre
    void huffman_free_tree(HuffmanNode* root) {
        if(root == nullptr) return;
        huffman_free_tree(root->left);
        huffman_free_tree(root->right);
        delete root;
    }

    HuffmanResult huffman_encode(const std::string& input) {
        if (input.empty()) {
            throw std::invalid_argument("Input string cannot be empty");
        }

        // etape 1 : calculer les frequences
        std::map<char, int> frequencies = compute_frequencies(input);

        // etape 2 : construire l'arbre de Huffman
        HuffmanNode* root = build_tree(frequencies);

        // etape 3 : construire la table des codes
        std::map<char, std::string> codebook;
        build_codebook(root, "", codebook);

        // etape 4 : encode la chaine
        std::string encoded = "";
        for ( char c : input) {
            encoded += codebook[c];
        }

        // liberer la memoire de l'arbre
        huffman_free_tree(root);
        return {encoded, codebook, frequencies};
    }

    std::string huffman_decode(const std::string& encoded, const std::map<char, std::string>& codebook) {
        if (encoded.empty()) {
            throw std::invalid_argument("encoded string cannot be empty");
        }
        if (codebook.empty()) {
            throw std::invalid_argument("codebook cannot be empty");
        }

        // inverser la table des codes : "0" -> 'A', "100" -> 'B'
        std::map<std::string, char> reverse_codebook;
        for (const auto& pair : codebook) {
            reverse_codebook[pair.second] = pair.first;
        }

        // parcourir la chaine binaire et identifier les symboles
        std::string decoded = "";
        std::string current = "";

        for (char bit : encoded) {
            current += bit;
            if(reverse_codebook.count(current)) {
                decoded += reverse_codebook[current];
                current = "";
            }
        }

        if (!current.empty()) {
            throw std::runtime_error("invalide encoded string: incomplete code at end");
        }

        return decoded;
    }
} // namespace algozip