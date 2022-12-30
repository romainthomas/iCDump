#!/usr/bin/sh
set -ex
python $GITHUB_WORKSPACE/bindings/python/setup.py --ninja --osx-arch=arm64 \
                  --lief-dir=/tmp/third-party/LIEF-0.13.0-Darwin/share/LIEF/cmake \
                  --llvm-dir=/tmp/third-party/LLVM-14.0.6-Darwin/lib/cmake/llvm \
                  build --build-temp=/tmp/arm64 bdist_wheel --skip-build \
                  --plat-name=macosx_${MACOSX_DEPLOYMENT_TARGET}_arm64

