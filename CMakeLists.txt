cmake_minimum_required(VERSION 3.16)
project(AlgoZip VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Sources des algorithmes
set(ALGO_SOURCES
    src/algorithms/bwt/bwt.cpp
    src/algorithms/huffman/huffman.cpp
    src/algorithms/lz77/lz77.cpp
    src/algorithms/arithmetic/arithmetic.cpp
    src/pipelines/bzip2_pipeline.cpp
    src/pipelines/lzma_pipeline.cpp
    src/utils/file_io.cpp
)

# Executable principal
add_executable(algozip
    src/main.cpp
    ${ALGO_SOURCES}
)

target_include_directories(algozip PRIVATE src)

# Tests unitaires
enable_testing()

add_executable(test_unit
    tests/unit/test_bwt.cpp
    tests/unit/test_huffman.cpp
    tests/unit/test_lz77.cpp
    tests/unit/test_arithmetic.cpp
    ${ALGO_SOURCES}
)

target_include_directories(test_unit PRIVATE src)
target_link_libraries(test_unit gtest gtest_main)

# Tests integration
add_executable(test_integration
    tests/integration/test_bzip2_pipeline.cpp
    tests/integration/test_lzma_pipeline.cpp
    ${ALGO_SOURCES}
)

target_include_directories(test_integration PRIVATE src)
target_link_libraries(test_integration gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(test_unit)
gtest_discover_tests(test_integration)
