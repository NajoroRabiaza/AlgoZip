#include <gtest/gtest.h>
#include <stdexcept>
#include "algorithms/bwt/bwt.hpp"

using namespace algozip;

// Tests encode
// valeurs attendues tirer directement du cours BWT

TEST(BWTEncode, BANANA) {
    BWTResult result = bwt_encode("BANANA");
    EXPECT_EQ(result.transformed, "NNBAAA");
    EXPECT_EQ(result.index, 3);
}

TEST(BWTEncode, SingleCharacter) {
    BWTResult result = bwt_encode("A");
    EXPECT_EQ(result.transformed, "A");
    EXPECT_EQ(result.index, 0);
}

TEST(BWTEncode, EmptyStringThrows) {
    EXPECT_THROW(bwt_encode(""), std::invalid_argument);
}

// Tests decode

TEST(BWTDecode, BANANA) {
    std::string result = bwt_decode("NNBAAA", 3);
    EXPECT_EQ(result, "BANANA");
}

TEST(BWTDecode, SingleCharacter) {
    std::string result = bwt_decode("A", 0);
    EXPECT_EQ(result, "A");
}

TEST(BWTDecode, EmptyStringThrows) {
    EXPECT_THROW(bwt_decode("", 0), std::invalid_argument);
}

// test roundtrip : encode puis decode doit redonner l'original
// c'est la propriete fondamentale de tout algo sans perte

TEST(BWTRoundtrip, BANANA) {
    std::string original = "BANANA";
    BWTResult encoded = bwt_encode(original);
    std::string decoded = bwt_decode(encoded.transformed, encoded.index);
    EXPECT_EQ(decoded, original);
}

TEST(BWTRoundtrip, ABRACADABRA) {
    std::string original = "ABRACADABRA";
    BWTResult encoded = bwt_encode(original);
    std::string decoded = bwt_decode(encoded.transformed, encoded.index);
    EXPECT_EQ(decoded, original);
}

TEST(BWTRoundtrip, LongString) {
    std::string original = "ABBBTTAYFGCABRYUIUUIACADABRARRARRAD";
    BWTResult encoded = bwt_encode(original);
    std::string decoded = bwt_decode(encoded.transformed, encoded.index);
    EXPECT_EQ(decoded, original);
}