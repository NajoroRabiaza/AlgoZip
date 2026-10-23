#ifndef SEVENZIP_PIPELINE_HPP
#define SEVENZIP_PIPELINE_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace algozip {

// Pipeline 7zip : LZ77 + Huffman
// LZ77 identifie les repetitions et produit des triplets
// Huffman compresse la representation de ces triplets
// C'est le principe de DEFLATE utilise dans 7-zip

std::vector<uint8_t> sevenzip_compress(const std::string& input);
std::string sevenzip_decompress(const std::vector<uint8_t>& compressed);

} // namespace algozip

#endif // SEVENZIP_PIPELINE_HPP
