#include <algorithm>
#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./sort.dfg.h"

#define N 64

int64_t a[N], b[N];
int64_t c[2*N];

// TODO: I do not understand restrict keyword, why don't I copy c?
void merge_impl() {
  SS_LINEAR_READ(a, N * 8, P_sort_A);
  SS_CONST(P_sort_A, SENTINAL, 1);
  SS_LINEAR_READ(b, N * 8, P_sort_B);
  SS_CONST(P_sort_B, SENTINAL, 1);
  SS_LINEAR_WRITE(P_sort_O, c, 2*N * 8);
  SS_WAIT_ALL();
}

void vanilla_merge_sort() {
  int i=0, j=0, iout=0;
  while (i < N && j < N) {
    if (a[i] <= b[j]) {
      c[iout] = a[i];
      ++i;
    } else {
      c[iout] = b[j];
      ++j;
    }
    ++iout;
  }
  printf("First 4 elements from the vanilla merge: %d, %d, %d, %d\n", c[0], c[1], c[2], c[3]);
}

int main() {
  for (int i = 0; i < N; ++i) {
    if(i > 0) {
      a[i] = a[i-1] + rand();
    } else {
      a[i] = rand();
    }
    if(i > 0) {
      b[i] = b[i-1] + rand();
    } else {
      b[i] = rand();
    }
  }

  // Warm up the I-cache
  SS_CONFIG(sort_config, sort_size);
  merge_impl();
  // Wrap around the region of interest
  begin_roi();
  merge_impl();
  end_roi();
  printf("First 4 elements from the vanilla merge: %d, %d, %d, %d\n", c[0], c[1], c[2], c[3]);
  // Dump the simulation log
  sb_stats();
  return 0;
}
