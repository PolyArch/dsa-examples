#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./dotp.dfg.h"
#define DTYPE int64_t
#define sentinel SENTINAL

// after a particular size, activity ratio is reducing?
#define N 1024 // 4096 // vector dim
#define M 64 // matrix rows
#define dens 1

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
int dwidth = sizeof(DTYPE);

void dotp_impl_16bit(int row) {


}

// TODO: access size is determining the size of the request somehow?
void dotp_impl(int row, int row_len) {
  
  SS_DMA_READ(&matrix_row_ind[row_offset[row]], 0, dwidth * row_len, 1, P_dotp_indA);
  SS_CONST(P_dotp_indA, sentinel, 1);

  SS_DMA_READ(&vector_ind[0], 0, dwidth * vec_len, 1, P_dotp_indB);
  SS_CONST(P_dotp_indB, sentinel, 1);

  SS_DMA_READ(&matrix_row_val[row_offset[row]], 0, dwidth * row_len, 1, P_dotp_valA);
  SS_CONST(P_dotp_valA, 0, 1);

  SS_DMA_READ(&vector_val[0], 0, dwidth * vec_len, 1, P_dotp_valB);
  SS_CONST(P_dotp_valB, 0, 1);
  
  SS_DMA_WRITE(P_dotp_O, 0, dwidth, 1, &output[row]);
  SS_WAIT_ALL();
}

// TODO: insert sentinel in the data (2D linear+const stream (if 3-D
// possible?), and make it streaming read)
// FIXME: why page fault in this case?
void spmv() {
  SS_CONFIG(dotp_config, dotp_size);
  dotp_impl(0, (row_offset[1]-row_offset[0]));
  // Streams are too small? To solve the problem, we can add sentinel in the
  // data, and load the overall stream.
// #if DECOMPOSABILITY==1
//   dotp_impl_16bit(0);
// #else
//   dotp_impl(0);
// #endif
//  for (int row = 0; row < M; ++row) {
//    dotp_impl(row);
//  }
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

  // data generation with a given density
  srand(0);
  vec_len=0;
  int stride = 1/dens;
  for (int j = 0; j < N; j+=stride) {
    vector_ind.push_back(j + rand()%stride);
    vector_val.push_back(1 + rand()%100);
    ++vec_len;
  }

  row_offset[0]=0;
  int nnz=0;
  for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; j += stride) {
      matrix_row_ind.push_back(j + rand()%stride);
      matrix_row_val.push_back(1 + rand()%100);
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
