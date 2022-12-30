#!/usr/bin/sh
set -ex

cp /third-party/LIEF-0.13.0-Linux-x86_64.tar.gz /tmp/
cp /third-party/LLVM-14.0.6-Linux-slim.tar.gz /tmp/

cd /tmp
tar xzvf LIEF-0.13.0-Linux-x86_64.tar.gz
tar xzvf LLVM-14.0.6-Linux-slim.tar.gz

export CXXFLAGS='-ffunction-sections -fdata-sections -fvisibility-inlines-hidden -static-libgcc -fvisibility=hidden'
export CFLAGS='-ffunction-sections -fdata-sections -static-libgcc'
export LDFLAGS='-Wl,--gc-sections'

cd /icdump/bindings/python

$PYTHON_BINARY setup.py --ninja \
               --lief-dir=/tmp/LIEF-0.13.0-Linux-x86_64/share/LIEF/cmake \
               --llvm-dir=/tmp/LLVM-14.0.6-Linux/lib/cmake/llvm \
               build \
               bdist_wheel --skip-build --dist-dir wheel_stage

find wheel_stage -iname "*-cp${PYTHON_VERSION}-*" -exec auditwheel repair -w dist --plat manylinux_2_27_x86_64 {} \;

chown -R 1000:1000 build dist wheel_stage

