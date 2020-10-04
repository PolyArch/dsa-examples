#include <complex>
#include <algorithm>
#include <stdio.h>

#include "sim_timing.h"

using std::complex;
using std::swap;

#ifndef N
#define N 1024
#endif

#ifndef U
#define U 4
#endif

void work(int blocks, double *a, double *b, double *w) {
  #pragma ss config
  {
    int span = N / blocks;
    double *aa = a + blocks;
    #pragma ss stream
    for (int64_t j = 0; j < N / 2; j += blocks) {
      #pragma ss dfg dedicated unroll(U)
      for (int64_t i = 0; i < blocks; ++i) {
        double &L = a[2 * j + i];
        double &R = aa[2 * j + i];
        double tmp(w[j] * L);
        b[i + j] = L + tmp;
        b[i + j + span / 2 * blocks] = R - tmp;
      }
    }
  }
}

int fft(double *a, double *b, double *w) {
  int cnt = 0;
  for (int blocks = N / 2; blocks; blocks >>= 1) {
    work(blocks, a, b, w);
  }
  return cnt;
}

double a[N], b[N], w[N];

int main() {
  fft(a, b, w);
  begin_roi();
  fft(a, b, w);
  end_roi();
  sb_stats();
}
