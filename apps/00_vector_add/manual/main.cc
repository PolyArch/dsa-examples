#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./add.dfg.h"

#define N 128

int64_t a[N], b[N], c[N];

void kernel(int64_t* __restrict a, int64_t* __restrict b, int64_t* __restrict c) {
  SS_CONFIG(add_config, add_size);
  SS_DMA_READ(a, 0, sizeof a, 1, P_add_A);
  SS_DMA_READ(b, 0, sizeof b, 1, P_add_B);
  SS_DMA_WRITE(P_add_C, 0, sizeof c, 1, c);
  SS_WAIT_ALL();
}

int main() {
  for (int i = 0; i < N; ++i) {
    a[i] = rand();
    b[i] = rand();
  }
  // Warm up the I-cache
  kernel(a, b, c);
  // Wrap around the region of interest
  begin_roi();
  kernel(a, b, c);
  end_roi();
  // Dump the simulation log
  sb_stats();
  return 0;
}
