#include <string.h>
#include <iostream>
#include <complex>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> 

#define PI 3.14159265358979303
#define N 512

using std::complex;

complex<float> a[N], _a[N], w[N];
complex<float> b[N], _b[N];

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./compute.dfg.h"
#include "./fine1.dfg.h"
#include "./fine2.dfg.h"

using std::complex;

complex<float> *fft(complex<float> *from, complex<float> *to, complex<float> *w) {

  complex<float> *_w = w + N - 2;

  SS_CONFIG(compute_config, compute_size);

  int blocks = N / 2;
  int span = N / blocks;

  int scr = 0;

  while (blocks >= 4) {

    SS_SCR_PORT_STREAM(scr             , 2 * blocks * 8, blocks * 8, span / 2, P_compute_L);
    SS_SCR_PORT_STREAM(scr + blocks * 8, 2 * blocks * 8, blocks * 8, span / 2, P_compute_RR);
    SS_REPEAT_PORT(blocks / 4);
    SS_DMA_READ(_w, 0, 4 * span, 1, P_compute_W);

    scr ^= 8192;
    SS_SCR_WRITE(P_compute_A, N * 4, scr);
    SS_SCR_WRITE(P_compute_B, N * 4, scr + N * 4);
    SS_WAIT_SCR_WR();

    blocks >>= 1;
    span <<= 1;
    _w -= span / 2;
  }

  SS_WAIT_ALL();
  
  SS_CONFIG(fine2_config, fine2_size);
  SS_SCRATCH_READ(scr, 8 * N, P_fine2_V);
  SS_DMA_READ(_w, 0, N * 2, 1, P_fine2_W);
  scr ^= 8192;
  SS_SCR_WRITE(P_fine2_A, N * 4, scr);
  SS_SCR_WRITE(P_fine2_B, N * 4, scr + N * 4);
  SS_WAIT_ALL();

  SS_CONFIG(fine1_config, fine1_size);
  SS_DMA_READ(w, 8, 8, N / 2, P_fine1_W);
  SS_SCRATCH_READ(scr, N * 8, P_fine1_V);
  SS_DMA_WRITE(P_fine1_A, 0, 4 * N, 1, to);
  SS_DMA_WRITE(P_fine1_B, 0, 4 * N, 1, to + N / 2);
  SS_WAIT_ALL();

  return to;
}

int main() {

  for (int i = 0; i < N / 2; ++i) {
    w[i] = complex<float>(cos(2 * PI * i / N), sin(2 * PI * i / N));
  }

  complex<float> *_w = w + N / 2;
  for (int i = 2; i < N; i <<= 1) {
    for (int j = 0; j < N / 2; j += i) {
      _w[j / i] = w[j];
    }
    _w += N / 2 / i;
  }

  fft(_a, _b, w);
  begin_roi();
  complex<float> *res = fft(a, b, w);
  end_roi();
  sb_stats();

  return 0;
}
