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
    constexpr int len = 4032 * 3024;
    float* input = new float[len];
    for (int i = 0; i < len; ++i) {
        input[i] = fRand(-1.f, 1.f);
    }
    float* output = new float[len];
    float* output1 = new float[len];
    // test
    auto time = std::chrono::system_clock::now();
    for (int i = 0; i < 100; ++i) {
        // NativeBoxFilter(input, 3, 4032, 3024, output);
        // Opt1BoxFilter(input, 3, 4032, 3024, output1);
        // Opt2BoxFilter(input, 3, 4032, 3024, output1);
        // Opt3BoxFilter(input, 3, 4032, 3024, output1);
        Opt4BoxFilter(input, 3, 4032, 3024, output1);
    }
    auto duration = duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time);
    auto time_end = double(duration.count()) * std::chrono::nanoseconds::period::num / std::chrono::nanoseconds::period::den;
    printf("\n -----| time: %f s \n", time_end / 100.0f);

    // for (int i = 0; i < len; i++) {
    //     printf("%f, %f \n", output[i], output1[i]);
    // }

    delete[] input;
    delete[] output;
    delete[] output1;
    return 0;
}
