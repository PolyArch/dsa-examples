#include <cmath>
#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"

#define N 128

double a[N];

void kernel(double* __restrict a) {
  double acc = 0.0;
  for (int i = 0; i < N; ++i) {
    acc += a[i] * a[i];
  }
  double norm;
  norm = 1.0 / sqrt(acc);
  for (int i = 0; i < N; ++i) {
    a[i] = a[i] * norm;
  }
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
