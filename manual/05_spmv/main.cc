#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./dotp.dfg.h"
// #include "./dotp_16.dfg.h"

#define DTYPE uint64_t
#define sentinel SENTINAL

// #define DTYPE uint16_t
// #define sentinel SENTINAL16

#define N 4096 // vector dim
#define M 64 // matrix rows
#define dens 0.1

// input matrix -- CSR format
std::vector<DTYPE> matrix_row_ind;
std::vector<DTYPE> matrix_row_val;
int row_offset[M+1];

// input vector -- CSR format
std::vector<DTYPE> vector_ind;
std::vector<DTYPE> vector_val;
int vec_len;

// output vector -- dense format
DTYPE output[N];
uint8_t dwidth = 8;

void dotp_impl(int row) {
  SS_LINEAR_READ(&matrix_row_ind[row_offset[row]], (row_offset[row+1]-row_offset[row]) * dwidth, P_dotp_indA);
  SS_CONST(P_dotp_indA, sentinel, 1);

  SS_LINEAR_READ(&vector_ind[0], vec_len * dwidth, P_dotp_indB);
  SS_CONST(P_dotp_indB, sentinel, 1);

  // indirect access to the values of matched indices (unknown length)
  SS_CONST(P_dotp_matchValA, 0, 1);
  SS_CONST(P_dotp_matchValB, 0, 1);
  
  // configure indirect datawidth
  SS_CONFIG_INDIRECT(T64, T64, 8, 1);
  SS_INDIRECT(P_dotp_matchIndA, &matrix_row_val[row_offset[row]], N, P_dotp_matchValA);
  SS_CONFIG_INDIRECT(T64, T64, 8, 1);
  SS_INDIRECT(P_dotp_matchIndB, &vector_val[0], N, P_dotp_matchValB);

  // reset all streams when the output is accumulated
  SS_RECV(P_dotp_O, output[row]);
  SS_RESET();
  SS_WAIT_ALL();
}

void spmv() {
  SS_CONFIG(dotp_config, dotp_size);
  dotp_impl(0);
  /*SS_CONFIG(dotp_config, dotp_size);
  for (int row = 0; row < M; ++row) {
    dotp_impl(row);
  }*/
}

void vanilla_spmv() {
  int matches = 0;
  int rounds = 0;
  // for (int row = 0; row < M; ++row) {
  for (int row = 0; row < 1; ++row) {
    int i=0, j=0;
    int row_len = row_offset[row+1] - row_offset[row];
    while (i<vec_len && j<row_len) {
      if(vector_ind[i] < matrix_row_ind[j]) {
        ++i;
      } else if(vector_ind[i] > matrix_row_ind[j]) {
        ++j;
      } else {
        ++i; ++j;
        output[row] += vector_val[i] * matrix_row_val[j];
        ++matches;
      }
      ++rounds;
    }
  }
  printf("number of multiplications required: %d and round: %d\n", matches, rounds);
}

int main() {

  dwidth = sizeof(DTYPE);

  srand(0);
  // data generation with a given sparsity
  vec_len=0;
  int stride = 1/dens;
  for (int j = 0; j < N; j+=stride) {
    vector_ind.push_back(j + rand()%stride);
    vector_val.push_back(1+rand());
    ++vec_len;
  }

  row_offset[0]=0;
  int nnz=0;
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; j += stride) {
      matrix_row_ind.push_back(j + rand()%stride);
      matrix_row_val.push_back(1+rand());
      ++nnz;
    }
    row_offset[i+1] = nnz;
  }

  vanilla_spmv();
  printf("Starting spmv with density: %0.2f\n", dens);

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
