#/bin/bash -e
cmake -S . -B build
if [ $(uname) == "Darwin" ]; then
    cmake --build build -j$(sysctl -n hw.ncpu)
else
    cmake --build build -j$(nproc)
fi