#!/bin/bash
set -e

echo "========== AlgoZip Build Script =========="

echo "Installing system dependencies..."
apt-get update -qq
apt-get install -y cmake build-essential libgtest-dev

echo "Compiling AlgoZip..."
cd ..
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make algozip -j$(nproc)

echo "AlgoZip binary compiled successfully"
ls -lh algozip

echo "Installing Node.js dependencies..."
cd ../web
npm install

echo "Build complete. Starting server..."
npm start
