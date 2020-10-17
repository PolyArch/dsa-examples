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
#define dens 0.1

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
  
  SS_LINEAR_READ(&matrix_row_ind[row_offset[row]], dwidth * row_len, P_dotp_indA);
  SS_CONST(P_dotp_indA, sentinel, 1);

  SS_LINEAR_READ(&vector_ind[0], dwidth * vec_len, P_dotp_indB);
  SS_CONST(P_dotp_indB, sentinel, 1);

  SS_LINEAR_READ(&matrix_row_val[row_offset[row]], dwidth * row_len, P_dotp_valA);
  SS_CONST(P_dotp_valA, 0, 1);

  SS_LINEAR_READ(&vector_val[0], dwidth * vec_len, P_dotp_valB);
  SS_CONST(P_dotp_valB, 0, 1);
  
  SS_LINEAR_WRITE(P_dotp_O, &output[row], dwidth);
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
