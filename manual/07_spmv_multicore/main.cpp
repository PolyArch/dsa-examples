#include <iostream>
#include <pthread.h>
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "sim_timing.h"
#include "dsaintrin.h"

#include "./dotp.dfg.h"

#define DTYPE int64_t
#define sentinel SENTINAL
#define SENTINAL8 (((uint8_t)1)<<7)
#define sentinel8 SENTINAL8
#define NUM_THREADS 4
#define N (NUM_THREADS) // matrix dim
#define M 8192 // vector dim
#define dens 0.2 // FIXME: different densities do not work??

using namespace std;

// Barrier variable
pthread_barrier_t barr;

// input matrix -- CSR format
std::vector<DTYPE> matrix_row_ind;
std::vector<DTYPE> matrix_row_val;
DTYPE row_offset[N+1];

// input vector -- CSR format
std::vector<DTYPE> vector_ind;
std::vector<DTYPE> vector_val;
int vec_len;
uint8_t dwidth = 8;

// output vector -- dense format
DTYPE output[N];
uint64_t bdcast_mask=0;

void dotp_impl(int row) {
   // show "row" load imbalance here...
  if(row==0) {
    SS_LINEAR_READ(&vector_ind[0], vec_len * 8, P_IND_1);
    SS_REM_PORT(P_IND_1, vec_len * 8, bdcast_mask, P_dotp_indB);
  }
  SS_LINEAR_READ(&matrix_row_ind[row_offset[row]], (row_offset[row+1]-row_offset[row]) * dwidth, P_dotp_indA);
  SS_CONST(P_dotp_indA, sentinel, 1);

  // indirect access to the values of matched indices (unknown length)
  SS_CONST(P_dotp_matchValA, 0, 1);
  SS_CONST(P_dotp_matchValB, 0, 1);
  
  // configure indirect datawidth
  SS_CONFIG_INDIRECT(T64, T64, 8, 1);
  SS_INDIRECT(P_dotp_matchIndA, &matrix_row_val[row_offset[row]], M, P_dotp_matchValA);
  SS_CONFIG_INDIRECT(T64, T64, 8, 1);
  SS_INDIRECT(P_dotp_matchIndB, &vector_val[0], M, P_dotp_matchValB);

  // reset all streams when the output is accumulated
  SS_RECV(P_dotp_O, output[row]);
  SS_RESET();
  
  SS_WAIT_ALL();
  SS_GLOBAL_WAIT(NUM_THREADS);
  
}

// each thread works on a pre-assigned number of "matrix rows"
void *entry_point(void *threadid) {
  
   long tid = (long)threadid;

   // configure and inform core about the number of threads in the system (with
   // global wait, it stops)
   SS_CONFIG(dotp_config,dotp_size);
   SS_GLOBAL_WAIT(NUM_THREADS);

  
   // computation 
   SS_CONFIG(dotp_config,dotp_size);
   SS_GLOBAL_WAIT(NUM_THREADS);
   dotp_impl(tid);
   begin_roi();
   dotp_impl(tid);
   end_roi();
   sb_stats();
   // pthread_exit(NULL);
   return NULL;
}

int main() {
  for(int d=0; d<NUM_THREADS; ++d) {
    bdcast_mask = bdcast_mask | (1 << d);
  }

  // data generation with a given sparsity
  vec_len=0;
  int stride = 1/dens;
  for (int j = 0; j < M; j+=stride) {
    vector_ind.push_back(j + rand()%stride);
    vector_val.push_back(rand());
    ++vec_len;
  }
  vector_ind.push_back(sentinel);
  ++vec_len;
  row_offset[0]=0;
  int nnz=0;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < M; j += stride) {
      matrix_row_ind.push_back(j + rand()%stride);
      matrix_row_val.push_back(rand());
      ++nnz;
    }
    row_offset[i+1] = nnz;
  }

  // Barrier initialization
  pthread_barrier_init(&barr, NULL, NUM_THREADS);
  pthread_t threads[NUM_THREADS];
  int rc;
  long t;
  
  for (t = 0; t < NUM_THREADS; ++t) {
    cout << "In main: creating thread: " << t << endl;;
    rc = pthread_create(&threads[t], NULL, entry_point, (void *)t);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
	  return 0;
    }
  }

  for(int i = 0; i < NUM_THREADS; ++i) {
    if(pthread_join(threads[i], NULL)) {
  	printf("Could not join thread %d\n", i);
      return -1;
    }
  }

  return 0;
}

/*
int global_row=0;

// central demand-based allocation
void multicore_spmv_demand(int tid) {
  while(global_row<N) { // considering granularity as row
    dotp_impl(matrix_row_ind[row_offset[global_row]], row_offset[global_row+1]-row_offset[global_row], matrix_row_val[row_offset[global_row]], vector_ind, vec_len, vector_val, c);
    atomicAdd(global_row); // central task queue
  }
}
*/
