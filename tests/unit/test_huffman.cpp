#include <gtest/gtest.h>
#include <stdexcept>
#include "algorithms/huffman/huffman.hpp"

using namespace algozip;

// Tests encode
// valeures attendues => ABRACADABRA : A=0, B=100, R=101, C=110, D=111
// encode = 01001010110011101001010

TEST(HuffmanEncode, ABRACADABRA_EncodedLength) {
    HuffmanResult result = huffman_encode("ABRACADABRA");
    // 23 bits attendus d'apres le cours
    EXPECT_EQ(result.encoded.size(), 23);
}

TEST(HuffmanEncode, ABRACADABRA_CodebookSize) {
    HuffmanResult result = huffman_encode("ABRACADABRA");
    // 5 symboles distincts : A, B, C, D, R
    EXPECT_EQ(result.codebook.size(), 5);
}

TEST(HuffmanEncode, ABRACADABRA_AhasShortestCode) {
    HuffmanResult result = huffman_encode("ABRACADABRA");
    // A est le plus frequent (5 fois), il doit avoir le code le plus court
    std::string code_A = result.codebook.at('A');
    std::string code_B = result.codebook.at('B');
    std::string code_C = result.codebook.at('C');
    std::string code_D = result.codebook.at('D');
    std::string code_R = result.codebook.at('R');
    EXPECT_LT(code_A.size(), code_B.size());
    EXPECT_LT(code_A.size(), code_C.size());
    EXPECT_LT(code_A.size(), code_D.size());
}

TEST(HuffmanEncode, FrequenciesCorrect) {
    HuffmanResult result = huffman_encode("ABRACADABRA");
    EXPECT_EQ(result.frequencies.at('A'), 5);
    EXPECT_EQ(result.frequencies.at('B'), 2);
    EXPECT_EQ(result.frequencies.at('R'), 2);
    EXPECT_EQ(result.frequencies.at('C'), 1);
    EXPECT_EQ(result.frequencies.at('D'), 1);
}

TEST(HuffmanEncode, EmptyStringThrows) {
    EXPECT_THROW(huffman_encode(""), std::invalid_argument);
}

// Tests decode

TEST(HuffmanDecode, EmptyStringThrows) {
    std::map<char, std::string> codebook = {{'A', "0"}};
    EXPECT_THROW(huffman_decode("", codebook), std::invalid_argument);
}

TEST(HuffmanDecode, EmptyCodebookthrows) {
    std::map<char, std::string> empty;
    EXPECT_THROW(huffman_decode("010", empty), std::invalid_argument);
}

// Tests roundtrip
// decompress(compress(data)) == data

TEST(HuffmanRoundtrip, ABRACADABRA) {
    std::string original = "ABRACADABRA";
    HuffmanResult encoded = huffman_encode(original);
    std::string decoded = huffman_decode(encoded.encoded, encoded.codebook);
    EXPECT_EQ(decoded, original);
}

TEST(HuffmanRoundtrip, CALCUL) {
    std::string original = "CALCUL";
    HuffmanResult encoded = huffman_encode(original);
    std::string decoded = huffman_decode(encoded.encoded, encoded.codebook);
    EXPECT_EQ(decoded, original);
}

TEST(HuffmanRoundtrip, LongString) {
    std::string original = "ABBBTTAYFGCARBRYUIUUIACADABRARRARRAD";
    HuffmanResult encoded = huffman_encode(original);
    std::string decoded = huffman_decode(encoded.encoded, encoded.codebook);
    EXPECT_EQ(decoded, original);
}

TEST(HuffmanRoundtrip, SingleChar) {
    std::string original = "AAAA";
    HuffmanResult encoded = huffman_encode(original);
    std::string decoded = huffman_decode(encoded.encoded, encoded.codebook);
    EXPECT_EQ(decoded, original);
}