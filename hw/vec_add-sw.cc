#include <cstdio>
#include <cstdlib>
#include <cstdint>

#define N 12

int64_t a[N], b[N], c[N];

void kernel(int64_t* __restrict a, int64_t* __restrict b, int64_t* __restrict c) {
  for (int i = 0; i < N; ++i) {
    c[i] = a[i] + b[i];
  }
}

int main() {
  // Generate Input
  for (int i = 0; i < N; ++i) {
    a[i] = i;
    b[i] = i;
  }
  // Warm up the I-cache
  kernel(a, b, c);
  // Doing Vector Add in typical way
  kernel(a, b, c);
  return 0;
}
