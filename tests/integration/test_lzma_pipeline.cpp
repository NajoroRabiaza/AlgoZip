#include <gtest/gtest.h>
#include <stdexcept>
#include "pipelines/lzma_pipeline.hpp"

using namespace algozip;

TEST(LzmaPipeline, EmptyInputThrows) {
    EXPECT_THROW(lzma_compress(""), std::invalid_argument);
}

TEST(LzmaPipeline, Roundtrip_ABRACADABRA) {
    std::string original = "ABRACADABRA";
    auto compressed = lzma_compress(original);
    std::string decompressed = lzma_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST(LzmaPipeline, Roundtrip_CABRACADABRARRARRAD) {
    std::string original = "CABRACADABRARRARRAD";
    auto compressed = lzma_compress(original);
    std::string decompressed = lzma_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST(LzmaPipeline, Roundtrip_LongString) {
    std::string original = "ABBBTTAYFGCABRYUIUUIACADABRARRARRAD";
    auto compressed = lzma_compress(original);
    std::string decompressed = lzma_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST(LzmaPipeline, Roundtrip_RepetitiveData) {
    std::string original = "AAAAAABBBBBBCCCCCCDDDDDD";
    auto compressed = lzma_compress(original);
    std::string decompressed = lzma_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}