#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./add.dfg.h"

#define N 512

int64_t a[N], b[N], c[N];

void kernel(int64_t* __restrict a, int64_t* __restrict b, int64_t* __restrict c) {
  SS_CONFIG(add_config, add_size);
  SS_LINEAR_READ(a, 8 * N, P_add_A);
  // SS_LINEAR_READ(b, 8 * N, P_add_B);
  SS_LINEAR_WRITE(P_add_C, c, 8 * N);
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
