#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./join.dfg.h"
#define DTYPE int64_t
#define sentinel SENTINAL

#define N 4096 // vector dim
#define dens 0.1

// input vector -- CSR format
std::vector<DTYPE> indA;
int an;

std::vector<DTYPE> indB;
int bn;

// matched indices -- sized to maximum
DTYPE output[N];

void join_bad() {

  int dwidth = sizeof(DTYPE);
  uint64_t done_flag=0; // it maintains whether the computation is completed..

  SS_CONST(P_join_indA, indA[0], 1);
  SS_CONST(P_join_indB, indB[0], 1);

  SS_CONFIG_INDIRECT(T64, T64, 8, 1);
  SS_INDIRECT(P_join_nextIA, &indA[0], 2 * (N+1), P_join_indA);
  SS_CONFIG_INDIRECT(T64, T64, 8, 1);
  SS_INDIRECT(P_join_nextJB, &indB[0], 2 * (N+1), P_join_indB);

  SS_LINEAR_WRITE(P_join_matchedInd, &output[0], N * 8);

  SS_RECV(P_join_done, done_flag);
  SS_RESET();

  SS_WAIT_ALL();
}

void vanilla_join() {
  int i=0, j=0;
  int iout=0;
  while (i<an && j<bn) {
    if (indA[i] < indB[j]) {
      ++i;
    } else if (indA[i] > indB[j]) {
      ++j;
    } else {
      output[++iout] = indA[i];
      ++i; ++j;
    }
  }
  // printf("Output of dot product: %d\n", output);
}

int main() {

  srand(0);
  // data generation with a given sparsity
  an=0;
  int stride = 1/dens;
  for (int j = 0; j < N; j+=stride) {
    indA.push_back(j + rand()%stride);
    ++an;
  }
  indA.push_back(sentinel);
  ++an;

  bn=0;
  for (int j = 0; j < N; j+=stride) {
    indB.push_back(j + rand()%stride);
    ++bn;
  }
  indB.push_back(sentinel);
  ++bn;

  vanilla_join();

  // Warm up the I-cache
  SS_CONFIG(join_config, join_size);
  // join_bad();
  // printf("Computed dot product is: %d\n", output);
  // Wrap around the region of interest
  begin_roi();
  join_bad();
  end_roi();
  // Dump the simulation log
  sb_stats();
  return 0;
}
