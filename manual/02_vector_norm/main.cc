#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./compute.dfg.h"

#define N 128

double a[N];

void kernel(double* __restrict a) {
  // Fill your implementation here by following the incremental tutoring!
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
