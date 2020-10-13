/*
Based on algorithm described here:
http://www.cs.berkeley.edu/~mhoemmen/matrix-seminar/slides/UCB_sparse_tutorial_1.pdf
*/

#include <algorithm>
#include <cstdint>
#include <vector>
#include <cstdlib>

#include "sim_timing.h"

#ifndef N
#define N 32
#endif

#ifndef U1
#define U1 1
#endif

#ifndef U2
#define U2 1
#endif

#ifndef U3
#define U3 1
#endif

#ifndef U4
#define U4 1
#endif

#ifndef U5
#define U5 1
#endif

void
spmv(int64_t *val, int64_t *cols, int64_t *rowDelimiters, int64_t *vec, int64_t *out){
  #pragma ss config
  {
    int i = 0;
    for (; i < N; i += 4) {
      {
        int k = i;
        int64_t sum = 0;
        int tmp_begin = rowDelimiters[k];
        int tmp_end = rowDelimiters[k + 1];
        #pragma ss stream nonblock
        #pragma ss dfg dedicated unroll(U1)
        for (int j = tmp_begin; j < tmp_end; ++j){
          sum += val[j] * vec[cols[j]];
        }
        out[k] = sum;
      }
      {
        int k = i + 1;
        int64_t sum = 0;
        int tmp_begin = rowDelimiters[k];
        int tmp_end = rowDelimiters[k + 1];
        #pragma ss stream nonblock
        #pragma ss dfg dedicated unroll(U2)
        for (int j = tmp_begin; j < tmp_end; ++j){
          sum += val[j] * vec[cols[j]];
        }
        out[k] = sum;
      }
      {
        int k = i + 2;
        int64_t sum = 0;
        int tmp_begin = rowDelimiters[k];
        int tmp_end = rowDelimiters[k + 1];
        #pragma ss stream nonblock
        #pragma ss dfg dedicated unroll(U3)
        for (int j = tmp_begin; j < tmp_end; ++j){
          sum += val[j] * vec[cols[j]];
        }
        out[k] = sum;
      }
      {
        int k = i + 3;
        int64_t sum = 0;
        int tmp_begin = rowDelimiters[k];
        int tmp_end = rowDelimiters[k + 1];
        #pragma ss stream nonblock
        #pragma ss dfg dedicated unroll(U4)
        for (int j = tmp_begin; j < tmp_end; ++j){
          sum += val[j] * vec[cols[j]];
        }
        out[k] = sum;
      }
    }
    for (; i < N; ++i){
      int64_t sum = 0;
      int tmp_begin = rowDelimiters[i];
      int tmp_end = rowDelimiters[i + 1];
      #pragma ss stream nonblock
      #pragma ss dfg dedicated unroll(U5)
      for (int j = tmp_begin; j < tmp_end; ++j){
        sum += val[j] * vec[cols[j]];
      }
      out[i] = sum;
    }
  }
}

//void
//random_shuffle(int n, int64_t *a) {
//  for (int i = 0; i < n; ++i) {
//    int x = rand() % n;
//    int y = rand() % n;
//    a[x] ^= a[y];
//    a[y] ^= a[x];
//    a[x] ^= a[y];
//  }
//}

int64_t vals[N * N];
int64_t cols[N * N];
int64_t atomic[N];
int64_t rows[N + 1];
int64_t out[N], vec[N];

int main() {

  for (int i = 0; i < N; ++i)
    atomic[i] = i;

  int total = 0;
  for (int i = 0; i < N; ++i) {
    rows[i] = total;
    int n = 3 + rand() % 2;
	std::random_shuffle(atomic, atomic + N);
	std::sort(atomic, atomic + n);
    for (int j = 0; j < n; ++j) {
      cols[j + total] = atomic[j];
      vals[j + total] = rand();
    }
    total += n;
    vec[i] = rand();
  }
  rows[N] = total;

  spmv(&vals[0], &cols[0], rows, vec, out);
  begin_roi();
  spmv(&vals[0], &cols[0], rows, vec, out);
  end_roi();
  sb_stats();

  return 0;
}
