#include <cstdlib>
#include <cstdint>

#define N 128

int64_t a[N], b[N], c[N];

void kernel(int64_t* __restrict a, int64_t* __restrict b, int64_t* __restrict c) {
  #pragma ss config
  {
    #pragma ss stream
    #pragma ss offload
    for (int i = 0; i < N; ++i) {
      c[i] = a[i] + b[i];
    }
  }
}

int main() {
  for (int i = 0; i < N; ++i) {
    a[i] = rand();
    b[i] = rand();
  }
  kernel(a, b, c);
  begin_roi();
  kernel(a, b, c);
  end_roi();
  ss_stats();
  return 0;
}