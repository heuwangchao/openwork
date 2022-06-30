#include <arm_neon.h>

#include <algorithm>

// native 版本-暴力实现
void NativeBoxFilter(float *input, int radius, int height, int width, float *output) {
    for (int h = 0; h < height; ++h) {
        int height_sift = width * h;
        for (int w = 0; w < width; ++w) {
            int start_h = std::max(0, h - radius);
            int end_h = std::min(height - 1, h + radius);
            int start_w = std::max(0, w - radius);
            int end_w = std::min(width - 1, w + radius);

            float tmp = 0;
            for (int sh = start_h; sh <= end_h; ++sh) {
                for (int sw = start_w; sw <= end_w; ++sw) {
                    tmp += input[sh * width + sw];
                }
            }
            output[height_sift + w] = tmp;
        }
    }
}