#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./join.dfg.h"
#define DTYPE int64_t
#define sentinel (((uint64_t)1)<<63)

#define N 4096 // vector dim
#define dens 0.1

// input vector -- CSR format
std::vector<DTYPE> indA;
int an;

std::vector<DTYPE> indB;
int bn;

// matched indices -- sized to maximum
DTYPE output[N];

void join() {

  int dwidth = sizeof(DTYPE);
  uint64_t done_flag=0; // it maintains whether the computation is completed..


  /* Useful intrinsics:
   * Read array to port: SS_LINEAR_READ(addr, bytes, port);
   * Read constant value to port: SS_CONST(port, data, times);
   * Write array to port: SS_LINEAR_WRITE(port, bytes, addr);
   */

  // Fill in memory streams here
  // Stream 1: Linear read indA from memory along with "sentinel"

  // Stream 2: Linear read indB from memory along with "sentinel"

  // Stream 3: Write matching index stream from CGRA

  // After the implementation is done, use `./run.sh main.out' to execute the simulator.


  SS_RECV(P_join_done, done_flag);
  SS_RESET();

  SS_WAIT_ALL();
}

void vanilla_join() {
  int i=0, j=0;
  int iout=0;
  while (i<an && bn) {
    if (indA[i] < indB[j]) {
      ++i;
    } else if (indA[i] < indB[j]) {
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
    indA.push_back(j + rand()%stride); // TODO: this should be rand?
    ++an;
  }

  bn=0;
  for (int j = 1; j < N; j+=stride) {
    indB.push_back(j + rand()%stride);
    ++bn;
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
