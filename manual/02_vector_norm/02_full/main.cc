#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./compute.dfg.h"

#define N 128

double a[N];

void kernel(double* __restrict a) {
  SS_CONFIG(compute_config, compute_size);
  // Compute the norm
  SS_DMA_READ(a, 8, 8, N, P_compute_A);
  // NOTE: Streams detinated the same port will be enforced in the order they appear.
  SS_CONST(P_compute_SIGNAL, 1, N - 1);
  SS_CONST(P_compute_SIGNAL, 0, 1);
  // Forward the norm to the inverse
  SS_RECURRENCE(P_compute_O, P_compute_NORM, 1);
  // Normalize the vector by the inversed norm
  SS_REPEAT_PORT(N);
  SS_RECURRENCE(P_compute_INV, P_compute_COEF, 1);
  SS_DMA_READ(a, 8, 8, N, P_compute_V);
  SS_DMA_WRITE(P_compute_RES, 8, 8, N, a);
  SS_WAIT_ALL();
}

int main() {
  for (int i = 0; i < N; ++i) {
    a[i] = rand();
  }
  // Warm up the I-cache
  kernel(a);
  // Wrap around the region of interest
  begin_roi();
  kernel(a);
  end_roi();
  // Dump the simulation log
  sb_stats();
  return 0;
}
