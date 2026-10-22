#include <gtest/gtest.h>
#include <stdexcept>
#include "pipelines/bzip2_pipeline.hpp"

using namespace algozip;

TEST(Bzip2Pipeline, EmptyInputThrows) {
    EXPECT_THROW(bzip2_compress(""), std::invalid_argument);
}

TEST(Bzip2Pipeline, Roundtrip_BANANA) {
    std::string original = "BANANA";
    auto compressed = bzip2_compress(original);
    std::string decompressed = bzip2_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST(Bzip2Pipeline, Roundtrip_ABRACADABRA) {
    std::string original = "ABRACADABRA";
    auto compressed = bzip2_compress(original);
    std::string decompressed = bzip2_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST(Bzip2Pipeline, Roundtrip_LongString) {
    std::string original = "ABBBTTAYFGCABRYUIUUIACADABRARRARRAD";
    auto compressed = bzip2_compress(original);
    std::string decompressed = bzip2_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST(Bzip2Pipeline, CompressedSmallerThanOriginal) {
    // sur une chaine avec repetition, la compression doit reduire la taille
    std::string original(200, 'A');
    original+= std::string(200, 'B');
    original+= std::string(200, 'A');
    auto compressed = bzip2_compress(original);
    EXPECT_LT(compressed.size(), original.size());
}