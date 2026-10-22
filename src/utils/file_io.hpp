#ifndef FILE_IO_HPP
#define FILE_IO_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace algozip {

// Taille d'un bloc : 64KB
// Assez grand pour une compression efficace
// Assez petit pour ne pas exploser la RAM
static constexpr size_t BLOCK_SIZE = 1024;

// Lit un fichier par blocs et appelle callback pour chaque bloc
// callback : fonction appelee avec le contenu de chaque bloc
void read_file_blocks(
    const std::string& path,
    size_t block_size,
    std::function<void(const std::string&, bool)> callback
);

// Ecrit un vecteur d'octets dans un fichier
void write_file_bytes(
    const std::string& path,
    const std::vector<uint8_t>& data
);

// Lit un fichier binaire complet en vecteur d'octets
std::vector<uint8_t> read_file_bytes(const std::string& path);

// Ecrit une string dans un fichier
void write_file_string(
    const std::string& path,
    const std::string& data
);

// Lit un fichier complet en string
std::string read_file_string(const std::string& path);

} // namespace algozip

#endif // FILE_IO_HPP
