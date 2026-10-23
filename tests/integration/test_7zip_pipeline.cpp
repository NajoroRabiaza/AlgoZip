#include <gtest/gtest.h>
#include <stdexcept>
#include "pipelines/7zip_pipeline.hpp"

using namespace algozip;

TEST(SevenzipPipeline, EmptyInputThrows) {
    EXPECT_THROW(sevenzip_compress(""), std::invalid_argument);
}

TEST(SevenzipPipeline, Roundtrip_ABRACADABRA) {
    std::string original = "ABRACADABRA";
    auto compressed = sevenzip_compress(original);
    std::string decompressed = sevenzip_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST(SevenzipPipeline, Roundtrip_CABRACADABRARRARRAD) {
    std::string original = "CABRACADABRARRARRAD";
    auto compressed = sevenzip_compress(original);
    std::string decompressed = sevenzip_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST(SevenzipPipeline, Roundtrip_LongString) {
    std::string original = "ABBBTTAYFGCABRYUIUUIACADABRARRARRAD";
    auto compressed = sevenzip_compress(original);
    std::string decompressed = sevenzip_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}

TEST(SevenzipPipeline, Roundtrip_RepetitiveData) {
    std::string original = "AAAAAABBBBBBCCCCCCDDDDDD";
    auto compressed = sevenzip_compress(original);
    std::string decompressed = sevenzip_decompress(compressed);
    EXPECT_EQ(decompressed, original);
}
