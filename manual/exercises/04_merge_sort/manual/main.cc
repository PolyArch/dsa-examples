#include <algorithm>
#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./sort.dfg.h"

#define N 64

int64_t a[N], b[N];

void merge_impl(int64_t* __restrict a, int an, int64_t* __restrict b, int bn, int64_t* __restrict buffer) {
  SS_DMA_READ(a, 0, an * 8, 1, P_sort_A);
  SS_CONST(P_sort_A, SENTINAL, 1);
  SS_DMA_READ(b, 0, bn * 8, 1, P_sort_B);
  SS_CONST(P_sort_B, SENTINAL, 1);
  SS_DMA_WRITE(P_sort_O, 0, (an + bn) * 8, 1, buffer);
  SS_WAIT_ALL();
}

void merge_sort(int64_t* __restrict a) {
  SS_CONFIG(sort_config, sort_size);
  int64_t* to_sort = a;
  int64_t* buffer  = b;
  for (int block = 1; block * 2 <= N; block *= 2) {
    for (int i = 0; i + block < N; i += block * 2) {
	  merge_impl(to_sort + i, block, to_sort + block + i, block, buffer + i);
	}
	std::swap(to_sort, buffer);
  }
}

int main() {
  for (int i = 0; i < N; ++i)
    a[i] = rand();

  // Warm up the I-cache
  merge_sort(b);
  // Wrap around the region of interest
  begin_roi();
  merge_sort(a);
  end_roi();
  // Dump the simulation log
  sb_stats();
  return 0;
}
