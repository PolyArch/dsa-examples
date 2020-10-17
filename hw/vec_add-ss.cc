#include <cstdlib>
#include <cstdint>
// Stream Dataflow Intrinsic and Dataflow Graph
#include "dsaintrin.h"
#include "./add.dfg.h"

#define N 512

int64_t a[N], b[N], c[N];

void kernel(int64_t* __restrict a, int64_t* __restrict b, int64_t* __restrict c) {
  SS_CONFIG(add_config, add_size);
  SS_DMA_READ(a, 8, 8, N, P_add_A);
  SS_DMA_READ(b, 8, 8, N, P_add_B);
  SS_DMA_WRITE(P_add_C, 8, 8, N, c);
  SS_WAIT_ALL();
}

int main() {
  // Generate Input
  for (int i = 0; i < N; ++i) {
    a[i] = i;
    b[i] = i-1;
  }
  // Warm up the I-cache
  kernel(a, b, c);
  // Stream-dataflow Acceleration
  kernel(a, b, c);
  return 0;
}
