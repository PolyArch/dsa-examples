#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./norm.dfg.h"

#define N 128

double a[N];

void kernel(double* __restrict a) {
  SS_CONFIG(norm_config, norm_size);
  // Compute the norm
  SS_LINEAR_READ(a, 8 * N, P_norm_A);
  // NOTE: Streams detinated the same port will be enforced in the order they appear.
  SS_CONST(P_norm_SIGNAL, 1, N - 1);
  SS_CONST(P_norm_SIGNAL, 0, 1);
  // Forward the norm to the inverse
  SS_RECURRENCE(P_norm_O, P_norm_NORM, 1);
  // Normalize the vector by the inversed norm
  SS_REPEAT_PORT(N);
  SS_RECURRENCE(P_norm_INV, P_norm_COEF, 1);
  SS_LINEAR_READ(a, 8 * N, P_norm_V);
  SS_LINEAR_WRITE(P_norm_RES, a, 8 * N);
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
