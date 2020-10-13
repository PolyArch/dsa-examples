#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./dotp.dfg.h"
#define DTYPE int64_t
#define sentinel SENTINAL

#define N 512 // vector dim
#define M 64 // matrix rows
#define sp 0.1

// input matrix -- CSR format
std::vector<DTYPE> matrix_row_ind;
std::vector<DTYPE> matrix_row_val;
DTYPE row_offset[M+1];

// input vector -- CSR format
std::vector<DTYPE> vector_ind;
std::vector<DTYPE> vector_val;
int vec_len;

// output vector -- dense format
DTYPE output[N];

// TODO: automatic experiment for adding decomposability
void dotp_impl(int row) {
  int dwidth = sizeof(DTYPE);
  
  SS_DMA_READ(&matrix_row_ind[row_offset[row]], 0, (row_offset[row+1]-row_offset[row]) * dwidth, 1, P_dotp_indA);
  SS_CONST(P_dotp_indA, sentinel, 1);

  SS_DMA_READ(&vector_ind[0], 0, vec_len * dwidth, 1, P_dotp_indB);
  SS_CONST(P_dotp_indB, sentinel, 1);

  // indirect access to the values of matched indices (unknown length)
  SS_CONST(P_dotp_matchValA, 0, 1);
  SS_CONST(P_dotp_matchValB, 0, 1);
  
  // configure indirect datawidth
  SS_CONFIG_INDIRECT(T64, T64, dwidth, 1);
  SS_INDIRECT(P_dotp_matchIndA, &matrix_row_val[row*N], N, P_dotp_matchValA);
  SS_CONFIG_INDIRECT(T64, T64, dwidth, 1);
  SS_INDIRECT(P_dotp_matchIndB, &vector_val[0], N, P_dotp_matchValB);

  // reset all streams when the output is accumulated
  SS_RECV(P_dotp_O, output[row]);
  SS_RESET();
  SS_WAIT_ALL();
}

void spmv() {
  SS_CONFIG(dotp_config, dotp_size);
  // Streams are too small? To solve the problem, we can add sentinel in the
  // data, and load the overall stream.
  dotp_impl(0);
  // for (int row = 0; row < M; ++row) {
  //   dotp_impl(row);
  // }
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

  // TODO: to load this in sparse format, we can calculate the indices of the
  // matched values...
  for (int j = 0; j < N; ++j) {
    vector_val.push_back(2);
  }
  row_offset[0]=0;
  int nnz=0;
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; j += stride) {
      matrix_row_ind.push_back(j);
      // matrix_row_val.push_back(rand());
      ++nnz;
    }
    row_offset[i+1] = nnz;
  }

  // dense format
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; ++j) {
      matrix_row_val.push_back(2);
    }
  }
  // printf("Done allocating data\n");

  // Warm up the I-cache
  spmv();
  printf("Done performing 1 round\n");
  // Wrap around the region of interest
  begin_roi();
  spmv();
  end_roi();
  // Dump the simulation log
  sb_stats();
  return 0;
}

/*
void spmv_impl(int row_start, int row_end) {

  int dwidth = sizeof(DTYPE);
  // constant should be embededded in it
  SS_DMA_READ(&matrix_row_ind[row_offset[row_start]], 0, (row_offset[row_end+1]-row_offset[row_start]) * dwidth, 1, P_dotp_indA);
  // SS_CONST(P_dotp_indA, sentinel, 1);

  SS_DMA_READ(&vector_ind[0], 0, vec_len * dwidth, row_end-row_start, P_dotp_indB);
  // SS_CONST(P_dotp_indB, sentinel, 1);

  // indirect access to the values of matched indices (unknown length)
  for(int row=row_start; row<row_end; ++row) {
    SS_CONST(P_dotp_matchValA, 0, 1);
    SS_CONST(P_dotp_matchValB, 0, 1);
    
    // configure indirect datawidth
    SS_CONFIG_INDIRECT(T64, T64, dwidth, 1);
    SS_INDIRECT(P_dotp_matchIndA, &matrix_row_val[row*N], N, P_dotp_matchValA);
    SS_CONFIG_INDIRECT(T64, T64, dwidth, 1);
    SS_INDIRECT(P_dotp_matchIndB, &vector_val[0], N, P_dotp_matchValB);
  }
  // reset all streams when the output is accumulated
  SS_RECV(P_dotp_O, output[row_start]); // TODO: it should reset what? (just the indirect streams)
  SS_RESET();
  SS_WAIT_ALL();

}*/
