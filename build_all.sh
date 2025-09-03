#!/bin/bash

set -e

# Release build
echo "Building Release..."
conan install . --build=missing
cmake --preset conan-release
cmake --build --preset conan-release

# Debug builds with different sanitizers
sanitizers=("asan" "tsan" "msan" "ubsan")

for sanitizer in "${sanitizers[@]}"; do
   echo "Building Debug with $sanitizer..."
   conan install . --build=missing --settings=build_type=Debug --options=sanitizer="$sanitizer"
   cmake --preset conan-debug
   cmake --build --preset conan-debug
   
   # Rename executables with sanitizer suffix
    cd build/Debug
    if [[ -f "TradingEngine" ]]; then
        sanitizer_clean="${sanitizer//+/_}"
        mv "TradingEngine" "TradingEngine_${sanitizer_clean}"
        echo "Renamed: TradingEngine -> TradingEngine_${sanitizer_clean}"
    fi
    cd ../..
done

echo ""
echo "All builds done!"
echo "Release executables: build/Release/"
echo "Debug executables with sanitizer suffixes: build/Debug/"
ls -la build/Debug/