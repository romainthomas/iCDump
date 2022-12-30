#!/usr/bin/sh
COMMIT=d8f42d74bfeb5d0a4d30472698e1fc0151f4844e
pushd /tmp
curl -LO https://github.com/lief-project/LIEF/archive/$COMMIT.tar.gz
tar xzvf $COMMIT.tar.gz

export CXXFLAGS="-ffunction-sections -fdata-sections -fvisibility-inlines-hidden -fvisibility=hidden"

cmake -GNinja -S /tmp/LIEF-$COMMIT -B /tmp/build_lief \
      -DCMAKE_CXX_COMPILER=clang++-11                 \
      -DCMAKE_C_COMPILER=clang-11                     \
      -DCMAKE_CXX_FLAGS="${CXXFLAGS}"                 \
      -DCMAKE_BUILD_TYPE=Release                      \
      -DLIEF_ELF=off                                  \
      -DLIEF_PE=off                                   \
      -DLIEF_OAT=off                                  \
      -DLIEF_VDEX=off                                 \
      -DLIEF_ART=off                                  \
      -DLIEF_DEX=off                                  \
      -DLIEF_ENABLE_JSON=off                          \
      -DLIEF_EXAMPLES=off

ninja -C /tmp/build_lief package
# cp /tmp/build_lief/LIEF-0.13.0-Linux-x86_64.tar.gz /LLVM/
