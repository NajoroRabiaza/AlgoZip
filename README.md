# AlgoZip

A functional clone of bzip2, lzma and 7-zip compression utilities,
built from scratch in C++ using fundamental compression algorithms.

## Algorithms implemented

- BWT (Burrows-Wheeler Transform)
- Huffman coding
- LZ77 (Lempel-Ziv 1977)
- Arithmetic coding

## Pipelines

| Mode  | Pipeline              | Description                        |
|-------|-----------------------|------------------------------------|
| bzip2 | BWT + Huffman         | Clone of the bzip2 utility         |
| lzma  | LZ77 + Arithmetic     | Clone of the lzma/7-zip utility    |

## Build

```bash
git clone git@github.com:NajoroRabiaza/AlgoZip.git
cd AlgoZip
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make algozip
```

## Usage

```bash
# Compress a file
./build/algozip compress --mode bzip2 input.txt output.az
./build/algozip compress --mode lzma  input.txt output.az

# Decompress a file
./build/algozip decompress --mode bzip2 output.az result.txt
./build/algozip decompress --mode lzma  output.az result.txt
```

## Run tests

```bash
cd build
make test_unit
./test_unit

make test_integration
./test_integration
```

## Test results

```
BWT         :  9 tests passed
Huffman     : 11 tests passed
LZ77        : 13 tests passed
Arithmetic  : 15 tests passed
bzip2       :  5 tests passed
lzma        :  5 tests passed
Total       : 58 tests passed
```

## Project structure

```
AlgoZip/
├── src/
│   ├── main.cpp
│   ├── algorithms/
│   │   ├── bwt/
│   │   ├── huffman/
│   │   ├── lz77/
│   │   └── arithmetic/
│   ├── pipelines/
│   │   ├── bzip2_pipeline
│   │   └── lzma_pipeline
│   └── utils/
└── tests/
    ├── unit/
    └── integration/
```

## Author

NajoroRabiaza