#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./join.dfg.h"
#define DTYPE int64_t
#define sentinel SENTINAL

#define N 128 // vector dim
#define dens 0.1

// input vector -- CSR format
std::vector<DTYPE> vector1_ind;
int vector1_len;

std::vector<DTYPE> vector2_ind;
int vector2_len;

// output vector -- dense format
DTYPE output[N];

void join() {
    /* Hint:
   * Possible useful indirect intrinsics:
   * 1. SS_INDIRECT(ind_port, addr, n, port)   
   *
   * ptr = addr;
   * for (int i = 0; i < n; ++i) {
   *   send ptr [ind_port_stream] to port;
   * }
   *
   * 2.  SS_CONFIG_INDIRECT(itype, dtype, mult, n)
   *     Sets the datatype of index, and output.
   *     Multiplier is the mult of the index
   *
   * 3. SS_RECV(port, mem_addr);
   *    Barrier until we write a value from port to mem_addr
   *
   * 4. SS_RESET();   
   * Reset all live requests and streams except CGRA data and configuration
   *
   */

  int dwidth = sizeof(DTYPE);
  
  SS_DMA_READ(&vector1_ind[0], 0, vector1_len * dwidth, 1, P_join_indA);
  SS_CONST(P_join_indA, sentinel, 1);

  SS_DMA_READ(&vector2_ind[0], 0, vector2_len * dwidth, 1, P_join_indB);
  SS_CONST(P_join_indB, sentinel, 1);
  
  SS_DMA_WRITE(P_join_matchedInd, 0, N * dwidth, 1, output)

  // reset all streams when the output is accumulated
  uint64_t done_flag;
  SS_RECV(P_join_done, done_flag);
  SS_RESET();

  SS_WAIT_ALL();
}

void vanilla_join() {
  int i=0, j=0;
  int iout=0;
  while (i<vector1_len && vector2_len) {
    if (vector1_ind[i] < vector2_ind[j]) {
      ++i;
    } else if (vector1_ind[i] < vector2_ind[j]) {
      ++j;
    } else {
      output[++iout] = vector1_ind[i];
      ++i; ++j;
    }
  }
  // printf("Output of dot product: %d\n", output);
}

int main() {

  srand(0);
  // data generation with a given sparsity
  vector1_len=0;
  int stride = 1/dens;
  for (int j = 0; j < N; j+=stride) {
    vector1_ind.push_back(j + rand()%stride); // TODO: this should be rand?
    ++vector1_len;
  }

  vector2_len=0;
  for (int j = 1; j < N; j+=stride) {
    vector2_ind.push_back(j + rand()%stride);
    ++vector2_len;
  }

  vanilla_join();

  // Warm up the I-cache
  SS_CONFIG(join_config, join_size);
  join();
  // printf("Computed dot product is: %d\n", output);
  // Wrap around the region of interest
  begin_roi();
  join();
  end_roi();
  // Dump the simulation log
  sb_stats();
  return 0;
}
