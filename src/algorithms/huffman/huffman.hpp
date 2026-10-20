#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include <string>
#include <map>
#include <vector>

namespace algozip {
    // un noeud de l'arbre de Huffman
    // si c'est une feuille : left et right sont nullptr, symbol contient le caractere
    // si c'est un noueud interne : left et right pointe vers les fils
    struct HuffmanNode {
        char symbol;
        int frequency;
        HuffmanNode* left;
        HuffmanNode* right;

        // constructeur pour une feuille
        HuffmanNode(char s, int f) : symbol(s), frequency(f), left(nullptr), right(nullptr) {}
        
        // constructeur pour un noeud interne
        HuffmanNode(int f, HuffmanNode* l, HuffmanNode* r) : symbol('\0'),frequency(f), left(l), right(r) {}
    };

    // resultat de l'encodage Huffman
    struct HuffmanResult {
        std::string encoded; // la chaine binaire encoder
        std::map<char, std::string> codebook; // tables des codes : 'A' => "0"
        std::map<char, int> frequencies; // frequences des symboles
    };

    // encode une chaine avec le codage de Huffman
    // input : la chaine a encoder (ex: "ABRACADABRA")
    // retourne un HuffmanResult avec la chaine encoder et la table des codes
    HuffmanResult huffman_encode(const std::string& input);

    // decode une chaine binaire avec la table des codes de Huffman
    // encoded : la chaine binaire (ex: "01001010110011101001010")
    // codebook : la table des codes issues de huffman_encode
    // retourne la chaine originale (ex: "ABRACADABRA")
    std::string huffman_decode(const std::string& encoded, const std::map<char, std::string>& codebook);

    // liberer la memoire de l'arbre de Huffman
    void huffman_free_tree(HuffmanNode* root);
}

#endif