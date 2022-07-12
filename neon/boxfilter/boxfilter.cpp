#include <arm_neon.h>

#include <algorithm>
#include <iostream>
#include <vector>

// native 版本-暴力实现
void NativeBoxFilter(float *input, const int radius, const int height, const int width, float *output) {
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

// 行列分离，先计算列，再计算行，行间可并行
void Opt1BoxFilter(float *input, const int radius, const int height, const int width, float *output) {
    std::vector<float> tmp_buffer(height * width);
    for (int h = 0; h < height; ++h) {
        int stride = h * width;
        for (int w = 0; w < width; ++w) {
            int start_w = std::max(0, w - radius);
            int end_w = std::min(width - 1, w + radius);
            float tmp = 0;
            for (int i = start_w; i <= end_w; ++i) {
                tmp += input[stride + i];
            }
            tmp_buffer[stride + w] = tmp;
        }
    }
    for (int h = 0; h < height; ++h) {
        int stride = h * width;
        int start_h = std::max(0, h - radius);
        int end_h = std::min(height - 1, h + radius);
        for (int w = 0; w < width; ++w) {
            float tmp = 0;
            for (int i = start_h; i <= end_h; ++i) {
                tmp += tmp_buffer[i * width + w];
            }
            output[stride + w] = tmp;
        }
    }
}

// 行列分离，kernel内去掉边界，减少重复计算
void Opt2BoxFilter(float *input, const int radius, const int height, const int width, float *output) {
    std::vector<float> tmp_buffer(height * width);
    // raw
    for (int h = 0; h < height; ++h) {
        int stride = h * width;
        float tmp = 0;
        for (int i = 0; i < radius; ++i) {
            tmp += input[stride + i];
        }
        // head, add new element
        for (int i = 0; i <= radius; ++i) {
            tmp += input[stride + radius + i];
            tmp_buffer[stride + i] = tmp;
        }
        // middle, add new element, sub old
        for (int i = radius + 1; i <= width - radius - 1; ++i) {
            tmp += input[stride + i + radius];
            tmp -= input[stride + i - radius - 1];
            tmp_buffer[stride + i] = tmp;
        }
        // tail
        for (int i = width - radius; i < width; ++i) {
            tmp -= input[stride + i - radius - 1];
            tmp_buffer[stride + i] = tmp;
        }
    }
    // col
    for (int w = 0; w < width; ++w) {
        float tmp = 0;
        // head
        for (int i = 0; i < radius; ++i) {
            tmp += tmp_buffer[i * width + w];
        }
        for (int i = 0; i <= radius; ++i) {
            tmp += tmp_buffer[(i + radius) * width + w];
            output[i * width + w] = tmp;
        }
        // middle
        for (int i = radius + 1; i <= height - radius - 1; ++i) {
            tmp += tmp_buffer[(i + radius) * width + w];
            tmp -= tmp_buffer[(i - radius - 1) * width + w];
            output[i * width + w] = tmp;
        }
        // tail
        for (int i = width - radius; i < height; ++i) {
            tmp -= tmp_buffer[(i - radius - 1) * width + w];
            output[i * width + w] = tmp;
        }
    }
}

// 行列分离，kernel内去掉边界，减少重复计算，添加width过度矩阵，按行访存
void Opt3BoxFilter(float *input, const int radius, const int height, const int width, float *output) {
    std::vector<float> tmp_buffer(height * width);
    // raw
    for (int h = 0; h < height; ++h) {
        int stride = h * width;
        float tmp = 0;
        for (int i = 0; i < radius; ++i) {
            tmp += input[stride + i];
        }
        // head, add new element
        for (int i = 0; i <= radius; ++i) {
            tmp += input[stride + radius + i];
            tmp_buffer[stride + i] = tmp;
        }
        // middle, add new element, sub old
        for (int i = radius + 1; i <= width - radius - 1; ++i) {
            tmp += input[stride + i + radius];
            tmp -= input[stride + i - radius - 1];
            tmp_buffer[stride + i] = tmp;
        }
        // tail
        for (int i = width - radius; i < width; ++i) {
            tmp -= input[stride + i - radius - 1];
            tmp_buffer[stride + i] = tmp;
        }
    }

    std::vector<float> colsum(width);
    for (int i = 0; i < width; ++i) {
        colsum[i] = 0.0f;
    }
    // col
    for (int i = 0; i < radius; ++i) {
        int stride = i * width;
        for (int w = 0; w < width; ++w) {
            colsum[w] += tmp_buffer[stride + w];
        }
    }

    // head
    for (int i = 0; i <= radius; ++i) {
        int stride = i * width;
        for (int w = 0; w < width; ++w) {
            colsum[w] += tmp_buffer[stride + radius * width + w];
            output[stride + w] = colsum[w];
        }
    }
    // middle
    for (int i = radius + 1; i <= height - radius - 1; ++i) {
        int stride = i * width;
        for (int w = 0; w < width; ++w) {
            colsum[w] += tmp_buffer[stride + radius * width + w];
            colsum[w] -= tmp_buffer[stride - (radius + 1) * width + w];
            output[stride + w] = colsum[w];
        }
    }
    // tail
    for (int i = height - radius; i < height; ++i) {
        int stride = i * width;
        for (int w = 0; w < width; ++w) {
            colsum[w] -= tmp_buffer[stride - (radius + 1) * width + w];
            output[stride + w] = colsum[w];
        }
    }
}

// neon 优化
void Opt4BoxFilter(float *input, const int radius, const int height, const int width, float *output) {
    std::vector<float> tmp_buffer(height * width);
    // raw
    for (int h = 0; h < height; ++h) {
        int stride = h * width;
        float tmp = 0;
        for (int i = 0; i < radius; ++i) {
            tmp += input[stride + i];
        }
        // head, add new element
        for (int i = 0; i <= radius; ++i) {
            tmp += input[stride + radius + i];
            tmp_buffer[stride + i] = tmp;
        }
        // middle, add new element, sub old
        for (int i = radius + 1; i <= width - radius - 1; ++i) {
            tmp += input[stride + i + radius];
            tmp -= input[stride + i - radius - 1];
            tmp_buffer[stride + i] = tmp;
        }
        // tail
        for (int i = width - radius; i < width; ++i) {
            tmp -= input[stride + i - radius - 1];
            tmp_buffer[stride + i] = tmp;
        }
    }

    std::vector<float> colsum(width);
    for (int i = 0; i < width; ++i) {
        colsum[i] = 0.0f;
    }
    // col
    for (int i = 0; i < radius; ++i) {
        float *colsum_ptr = colsum.data();
        float *tmp_buffer_ptr = tmp_buffer.data() + i * width;
        int w = 0;
        for (; w < width; w += 4) {
            float32x4_t tmp_colsum = vld1q_f32(colsum_ptr);
            float32x4_t tmp_buffer_opt = vld1q_f32(tmp_buffer_ptr);
            float32x4_t sum = vaddq_f32(tmp_colsum, tmp_buffer_opt);

            vst1q_f32(colsum_ptr, sum);
            colsum_ptr += 4;
            tmp_buffer_ptr += 4;
        }
        for (; w < width; ++w) {
            *colsum_ptr += *tmp_buffer_ptr;
            colsum_ptr++;
            tmp_buffer_ptr++;
        }
    }

    // head
    for (int i = 0; i <= radius; ++i) {
        int stride = i * width;
        float *output_ptr = output + stride;
        float *colsum_ptr = colsum.data();
        float *tmp_buffer_ptr = tmp_buffer.data() + stride + radius * width;
        int w = 0;
        for (; w < width; w += 4) {
            float32x4_t tmp_colsum = vld1q_f32(colsum_ptr);
            float32x4_t tmp_buffer_opt = vld1q_f32(tmp_buffer_ptr);
            float32x4_t sum = vaddq_f32(tmp_colsum, tmp_buffer_opt);

            vst1q_f32(colsum_ptr, sum);
            vst1q_f32(output_ptr, sum);

            output_ptr += 4;
            colsum_ptr += 4;
            tmp_buffer_ptr += 4;
        }
        for (; w < width; ++w) {
            *colsum_ptr += *tmp_buffer_ptr;
            *output_ptr = *colsum_ptr;
            output_ptr++;
            colsum_ptr++;
            tmp_buffer_ptr++;
        }
    }
    // middle
    for (int i = radius + 1; i <= height - radius - 1; ++i) {
        int stride = i * width;
        float *output_ptr = output + stride;
        float *colsum_ptr = colsum.data();
        float *tmp_buffer_newptr = tmp_buffer.data() + stride + radius * width;
        float *tmp_buffer_oldptr = tmp_buffer.data() + stride - (radius + 1) * width;

        int w = 0;
        for (; w < width; w += 4) {
            float32x4_t tmp_colsum = vld1q_f32(colsum_ptr);
            float32x4_t tmp_buffer_new = vld1q_f32(tmp_buffer_newptr);
            float32x4_t tmp_buffer_old = vld1q_f32(tmp_buffer_oldptr);
            float32x4_t sum = vaddq_f32(tmp_colsum, tmp_buffer_new);
            sum = vsubq_f32(sum, tmp_buffer_old);

            vst1q_f32(colsum_ptr, sum);
            vst1q_f32(output_ptr, sum);

            output_ptr += 4;
            colsum_ptr += 4;
            tmp_buffer_newptr += 4;
            tmp_buffer_oldptr += 4;
        }
        for (; w < width; ++w) {
            *colsum_ptr += *tmp_buffer_newptr;
            *colsum_ptr -= *tmp_buffer_oldptr;
            *output_ptr += *colsum_ptr;

            output_ptr++;
            colsum_ptr++;
            tmp_buffer_newptr++;
            tmp_buffer_oldptr++;
        }
    }
    // tail
    for (int i = height - radius; i < height; ++i) {
        int stride = i * width;
        float *output_ptr = output + stride;
        float *colsum_ptr = colsum.data();
        float *tmp_buffer_oldptr = tmp_buffer.data() + stride - (radius + 1) * width;

        int w = 0;
        for (; w < width; w += 4) {
            float32x4_t tmp_colsum = vld1q_f32(colsum_ptr);
            float32x4_t tmp_buffer_old = vld1q_f32(tmp_buffer_oldptr);
            float32x4_t sum = vsubq_f32(tmp_colsum, tmp_buffer_old);

            vst1q_f32(colsum_ptr, sum);
            vst1q_f32(output_ptr, sum);

            output_ptr += 4;
            colsum_ptr += 4;
            tmp_buffer_oldptr += 4;
        }
        for (; w < width; ++w) {
            *colsum_ptr -= *tmp_buffer_oldptr;
            *output_ptr += *colsum_ptr;

            output_ptr++;
            colsum_ptr++;
            tmp_buffer_oldptr++;
        }
    }
}
