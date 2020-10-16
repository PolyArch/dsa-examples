#include <cstdlib>
#include <cstdint>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./add.dfg.h"

#define N 512

int64_t a[N], b[N], c[N];

void kernel(int64_t* __restrict a, int64_t* __restrict b, int64_t* __restrict c) {
  /* Hint:
   * Possible usefule intrinsics:
   * 1. SS_LINEAR_READ(a, bytes, port);
   *   a[0:bytes] -> port
   * 
   * 2. SS_LINEAR_WRITE(port, a, bytes);
   *   port -> a[0:bytes]
   *
   * 3. SS_WAIT_ALL();
   *    Wait for everything happen on the spatial architecture to finish.
   */
  // Fill the kernel implementation here.
  // After the implementation is done, use `./run.sh main.out' to execute the simulator.
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
