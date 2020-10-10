#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./dotp_int16.dfg.h"
#define DTYPE int16_t
#define sentinel SENTINAL16


#define N 64 // matrix dim
#define M 64 // vector dim
#define sp 0.1

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
  int dwidth = sizeof(DTYPE);
  
  // join of indices
  SS_DMA_READ(&matrix_row_ind[row_offset[row]], 0, (row_offset[row+1]-row_offset[row]) * dwidth, 1, P_dotp_int16_indA);
  SS_CONST(P_dotp_int16_indA, sentinel, 1);

  SS_DMA_READ(&vector_ind[0], 0, vec_len * dwidth, 1, P_dotp_int16_indB);
  SS_CONST(P_dotp_int16_indB, sentinel, 1);

  // indirect access to the values of matched indices (unknown length)
  SS_CONST(P_dotp_int16_matchValA, 0, 1);
  SS_CONST(P_dotp_int16_matchValB, 0, 1);
  
  // configure indirect datawidth
  SS_CONFIG_INDIRECT(T16, T16, dwidth, 1);
  SS_INDIRECT(P_dotp_int16_matchIndA, &matrix_row_val[row*M], M, P_dotp_int16_matchValA);
  SS_CONFIG_INDIRECT(T16, T16, dwidth, 1);
  SS_INDIRECT(P_dotp_int16_matchIndB, &vector_val[0], M, P_dotp_int16_matchValB);

  // reset all streams when the output is accumulated
  SS_RECV(P_dotp_int16_O, output[row]);
  SS_RESET();
  SS_WAIT_ALL();
}

void spmv() {
  SS_CONFIG(dotp_int16_config, dotp_int16_size);
  // dotp_impl(0);
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
    // vector_val.push_back(rand());
    ++vec_len;
  }
  for (int j = 0; j < N; ++j) {
    vector_val.push_back(2);
  }
  row_offset[0]=0;
  int nnz=0;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < M; j += stride) {
      matrix_row_ind.push_back(j);
      // matrix_row_val.push_back(rand());
      ++nnz;
    }
    row_offset[i+1] = nnz;
  }

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < M; ++j) {
      matrix_row_val.push_back(2);
    }
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
