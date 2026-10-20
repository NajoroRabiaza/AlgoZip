#include <gtest/gtest.h>
#include <stdexcept>
#include "algorithms/lz77/lz77.hpp"

using namespace algozip;

// Tests encode
// valeurs attendus => CABRACADABRARRARRAD -> 8 triplets

TEST(LZ77Encode, EmptyStringThrows) {
    EXPECT_THROW(lz77_encode(""), std::invalid_argument);
}

TEST(LZ77Encode, CABRACADABRARRARRAD_TokenCount) {
    LZ77Result result = lz77_encode("CABRACADABRARRARRAD", 7, 6);
    // 8 triplet attendus d'apres le codage
    EXPECT_EQ(result.tokens.size(), 8);
}

TEST(LZ77Encode, CABRACADABRARRARRAD_FirstFourTokens) {
    LZ77Result result = lz77_encode("CABRACADABRARRARRAD", 7, 6);
    // les 4 premiers sont des emissions directes
    EXPECT_EQ(result.tokens[0].offset, 0);
    EXPECT_EQ(result.tokens[0].length, 0);
    EXPECT_EQ(result.tokens[0].next, 'C');

    EXPECT_EQ(result.tokens[1].offset, 0);
    EXPECT_EQ(result.tokens[1].length, 0);
    EXPECT_EQ(result.tokens[1].next, 'A');

    EXPECT_EQ(result.tokens[2].offset, 0);
    EXPECT_EQ(result.tokens[2].length, 0);
    EXPECT_EQ(result.tokens[2].next, 'B');

    EXPECT_EQ(result.tokens[3].offset, 0);
    EXPECT_EQ(result.tokens[3].length, 0);
    EXPECT_EQ(result.tokens[3].next, 'R');
}

TEST(LZ77Encode, CABRACADABRARRARRAD_Token5) {
    LZ77Result result = lz77_encode("CABRACADABRARRARRAD", 7, 6);
    // token 5 : (3, 1, C) d'apres le codage du cours toujours
    EXPECT_EQ(result.tokens[4].offset, 3);
    EXPECT_EQ(result.tokens[4].length, 1);
    EXPECT_EQ(result.tokens[4].next, 'C');
}

TEST(LZ77Encode, ABBBTTAYFGCABRYUIUUIACADABRARRARRAD_TokenCount) {
    // avec search=6, lookahead=6 : 22 triplets comme dans le cours
    LZ77Result result = lz77_encode("ABBBTTAYFGCABRYUIUUIACADABRARRARRAD", 7, 6);
    EXPECT_EQ(result.tokens.size(), 22);
}

// Test de decodage

TEST(LZ77Decode, EmptyTokensThrows) {
    std::vector<LZ77Token> tokens = {
        {0, 0, 'C'},
        {0, 0, 'A'},
        {0, 0, 'B'},
        {0, 0, 'R'},
        {3, 1, 'C'},
        {2, 1, 'D'},
        {7, 4, 'R'},
        {3, 5, 'D'},
    };
    std::string result = lz77_decode(tokens);
    EXPECT_EQ(result, "CABRACADABRARRARRAD");
}

// Test roundtrip

TEST(LZ77Roundtrip, CABRACADABRARRARRAD) {
    std::string original = "CABRACADABRARRARRAD";
    LZ77Result encoded = lz77_encode(original, 6, 6);
    std::string decoded = lz77_decode(encoded.tokens);
    EXPECT_EQ(decoded, original);
}

TEST(LZ77Roundtrip, ABBBTTAYFGCABRYUIUUIACADABRARRARRAD) {
    std::string original = "ABBBTTAYFGCABRYUIUUIACADABRARRARRAD";
    LZ77Result encoded = lz77_encode(original, 6, 6);
    std::string decoded = lz77_decode(encoded.tokens);
    EXPECT_EQ(decoded, original);
}

TEST(LZ77Roundtrip, ABRACADABRA) {
    std::string original = "ABRACADABRA";
    LZ77Result encoded = lz77_encode(original, 6, 6);
    std::string decoded = lz77_decode(encoded.tokens);
    EXPECT_EQ(decoded, original);
}