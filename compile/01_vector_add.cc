#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"

#define N 128

int64_t a[N], b[N], c[N];

void kernel(int64_t* __restrict a, int64_t* __restrict b, int64_t* __restrict c) {
  #pragma ss config
  {
    #pragma ss stream
    #pragma ss dfg dedicated
    for (int i = 0; i < N; ++i) {
      c[i] = a[i] + b[i];
    }
  }
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
