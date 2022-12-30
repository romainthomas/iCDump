#!/usr/bin/sh
set -ex

LLVM_TARGET="X86"

export CXXFLAGS="-ffunction-sections -fdata-sections -fvisibility-inlines-hidden -fvisibility=hidden"

cmake -GNinja -S /LLVM/llvm -B /tmp/build_llvm       \
      -DCMAKE_CXX_FLAGS="${CXXFLAGS}"                \
      -DCMAKE_CXX_COMPILER=clang++-11                \
      -DCMAKE_C_COMPILER=clang-11                    \
      -DCMAKE_BUILD_TYPE=Release                     \
      -DCMAKE_INSTALL_PREFIX=/llvm-install           \
      -DLLVM_ENABLE_LTO=OFF                          \
      -DLLVM_ENABLE_TERMINFO=OFF                     \
      -DLLVM_ENABLE_THREADS=ON                       \
      -DLLVM_USE_NEWPM=ON                            \
      -DLLVM_TARGET_ARCH=${LLVM_TARGET}              \
      -DLLVM_TARGETS_TO_BUILD=${LLVM_TARGET}         \
      -DLLVM_ENABLE_PROJECTS="clang;llvm"

ninja -C /tmp/build_llvm package
cp /tmp/build_llvm/LLVM-14.0.6-Linux.tar.gz /LLVM/
