#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./dfg0.dfg.h"
#include "./dfg1.dfg.h"
#include "./dfg2.dfg.h"

#define N 128

double a[N];

void kernel(double* __restrict a) {
  SS_CONFIG(dfg0_config, dfg0_size);
  // Compute the norm
  double b;
  for (int i = 0; i < N; ++i) {
    SS_DMA_READ(a + i, 8, 8, 1, P_dfg0_A);
    SS_DMA_READ(&b, 8, 8, 1, P_dfg0_B);
	SS_DMA_WRITE(P_dfg0_O, 8, 8, 1, &b);
	SS_WAIT_ALL();
  }

  SS_CONFIG(dfg1_config, dfg1_size);
  SS_DMA_READ(&b, 8, 8, 1, P_dfg1_NORM);
  SS_DMA_WRITE(P_dfg1_INV, 8, 8, 1, &b);
  SS_WAIT_ALL();

  SS_CONFIG(dfg2_config, dfg2_size);
  SS_DMA_READ(a, 8, 8, N, P_dfg2_V);
  SS_DMA_WRITE(P_dfg2_RES, 8, 8, N, a);
  for (int i = 0; i < N; ++i) {
    SS_DMA_READ(&b, 8, 8, 1, P_dfg2_COEF);
  }
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
