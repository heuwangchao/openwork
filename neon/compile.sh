#!/bin/bash

export ANDROID_NDK=~/tools/android-ndk-r23

rm -rf build/*
cd build

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DANDROID_ABI="arm64-v8a" \
    -DANDROID_STL=c++_static \
    -DANDROID_ARM_MODE=arm \
    -DANDROID_NATIVE_API_LEVEL=android-21 \
    -DANDROID_TOOLCHAIN=clang

make

cd ..