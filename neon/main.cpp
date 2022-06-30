#include <arm_neon.h>
#include <math.h>
#include <stddef.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <vector>

#include "./boxfilter/boxfilter.h"

static float fRand(float fMin, float fMax) {
    float f = (float)rand() / (float)RAND_MAX;
    return fMin + f * (fMax - fMin);
}

int main(int argc, char** argv) {
    constexpr int len = 32 * 32;
    float* input = new float[len];
    for (int i = 0; i < len; ++i) {
        input[i] = fRand(-1.f, 1.f);
    }
    float* output = new float[len];
    NativeBoxFilter(input, 5, 32, 32, output);

    for (int i = 0; i < len; i++) {
        printf("%f, ", output[i]);
    }

    delete[] input;
    delete[] output;
    return 0;
}
