# AlgoZip

A functional clone of bzip2, lzma and 7-zip compression utilities,
built from scratch in C++ using fundamental compression algorithms.

## Algorithms implemented

- BWT (Burrows-Wheeler Transform)
- Huffman coding
- LZ77 (Lempel-Ziv 1977)
- Arithmetic coding

## Pipelines

- bzip2 mode : BWT + Huffman
- lzma mode  : LZ77 + Arithmetic coding

## Usage

```bash
# Compress
./algozip compress --mode bzip2 input.txt output.az

# Decompress
./algozip decompress --mode bzip2 input.az output.txt
```

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Author

NajoroRabiaza
