#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./dotp.dfg.h"

#define DTYPE int64_t
#define sentinel SENTINAL
#define N 64 // matrix dim
#define M 64 // vector dim
#define sp 0.2

// input matrix -- CSR format
std::vector<DTYPE> matrix_row_ind;
std::vector<DTYPE> matrix_row_val;
DTYPE row_offset[N+1];

// input vector -- CSR format
std::vector<DTYPE> vector_ind;
std::vector<DTYPE> vector_val;
int vec_len;

// output vector -- dense format
DTYPE output[N];

// TODO: automatic experiment for adding decomposability
void dotp_impl(int row) {

  // join of indices
  SS_DMA_READ(&matrix_row_ind[row_offset[row]], 0, (row_offset[row+1]-row_offset[row]) * 8, 1, P_dotp_indA);
  SS_CONST(P_dotp_indA, sentinel, 1);

  SS_DMA_READ(&vector_ind[0], 0, vec_len * 8, 1, P_dotp_indB);
  SS_CONST(P_dotp_indB, sentinel, 1);

  // indirect access to the values of matched indices (unknown length)
  SS_INDIRECT(P_dotp_matchIndA, &matrix_row_val[row_offset[row]], M, P_dotp_matchValA);
  SS_INDIRECT(P_dotp_matchIndB, &vector_val[0], M, P_dotp_matchValB);

  // reset all streams when the output is accumulated
  SS_RECV(P_dotp_O, output[row]);
  SS_RESET();
  SS_WAIT_ALL();
}

void spmv() {
  SS_CONFIG(dotp_config, dotp_size);
  for (int row = 0; row < N; ++row) {
    dotp_impl(row);
  }
}

int main() {

  // data generation with a given sparsity
  vec_len=0;
  int stride = 1/sp;
  for (int j = 0; j < N; j+=stride) {
    vector_ind.push_back(j);
    vector_val.push_back(rand());
    ++vec_len;
  }
  row_offset[0]=0;
  int nnz=0;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < M; j += stride) {
      matrix_row_ind.push_back(j);
      matrix_row_val.push_back(rand());
      ++nnz;
    }
    row_offset[i+1] = nnz;
  }

  // Warm up the I-cache
  spmv();
  // Wrap around the region of interest
  begin_roi();
  spmv();
  end_roi();
  // Dump the simulation log
  sb_stats();
  return 0;
}