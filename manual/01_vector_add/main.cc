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
   * 1. SS_DMA_READ(addr, stride, size, n, port);
   *    port is some macro from add.dfg.h, P_add_*.
   *
   *    ptr = addr;
   *    for (int i = 0; i < n; ++i) {
   *      for (int j = 0; j < size; ++j) {
   *        send ptr[j] to port;
   *      }
   *      ptr += stride;
   *    }
   * 
   * 2. SS_DMA_WRITE(port, stride, size, n, addr);
   *    This intrinsic has very similar semantics compared with SS_DMA_READ,
   *    in term of the memory access pattern. The inner most loop body will be
   *    changed to write data from the port to the pointer.
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
