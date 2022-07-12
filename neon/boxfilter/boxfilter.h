#ifndef BOXFILTER_H
#define BOXFILTER_H

void NativeBoxFilter(float *input, const int radius, const int height, const int width, float *output);
void Opt1BoxFilter(float *input, const int radius, const int height, const int width, float *output);
void Opt2BoxFilter(float *input, const int radius, const int height, const int width, float *output);
void Opt3BoxFilter(float *input, const int radius, const int height, const int width, float *output);
void Opt4BoxFilter(float *input, const int radius, const int height, const int width, float *output);
#endif /* CPUINFO_H */
