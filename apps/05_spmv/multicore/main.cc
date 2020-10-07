#include <pthread.h>
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <vector>

#include "sim_timing.h"
#include "net_util_func.h"
#include "dsaintrin.h"

#include "./dotp.dfg.h"

#define DTYPE int64_t
#define sentinel SENTINAL
#define N 64 // matrix dim
#define M 64 // vector dim
#define sp 0.2

// Barrier variable
pthread_barrier_t barr;
#define NUM_THREADS 8

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

void dotp_impl(int row) {

  // join of indices
  SS_DMA_READ(&matrix_row_ind[row_offset[row]], 0, (row_offset[row+1]-row_offset[row]) * 8, 1, P_dotp_indA);
  SS_CONST(P_dotp_indA, sentinel, 1);

  // SS_DMA_READ(&vector_ind[0], 0, vec_len * 8, 1, P_dotp_indB);
  // SS_CONST(P_dotp_indB, sentinel, 1);

  // indirect access to the values of matched indices (unknown length)
  SS_INDIRECT(P_dotp_matchIndA, &matrix_row_val[row_offset[row]], M, P_dotp_matchValA);
  SS_INDIRECT(P_dotp_matchIndB, &vector_val[0], M, P_dotp_matchValB); // TODO: it might be better to broadcast it in this case...

  // reset all streams when the output is accumulated
  SS_RECV(P_dotp_O, output[row]);
  SS_RESET();
  SS_WAIT_ALL();
}

void broadcast_vector_ind() {
  uint64_t mask=0;
  for(int i=0; i<NUM_THREADS; ++i) {
    if(i!=0) addDest(mask,i);
  }
  SS_DMA_READ(&vector_ind[0], 0, vec_len * 8, 1, P_IND_1);
  SS_CONST(P_IND_1, sentinel, 1);
  SS_REM_PORT(P_IND_1, vec_len * 8, mask, P_dotp_indB);
  SS_WAIT_ALL();
}

// each thread works on a pre-assigned number of "matrix rows"
void *multicore_spmv(void *threadid) {
  
   long tid;
   tid = (long)threadid;

   // configure and inform core about the number of threads in the system
   SS_CONFIG(dotp_config,dotp_size);
   SS_GLOBAL_WAIT(NUM_THREADS);
 
   // Synchronization point
   int rc = pthread_barrier_wait(&barr);
   if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
   {
     printf("Could not wait on barrier\n");
     exit(-1);
   }

   if(tid==0) {
     broadcast_vector_ind();
   }
   int row=tid;
   while(row<N) { // considering granularity as row
     row += NUM_THREADS;
     dotp_impl(row);
   }
   SS_GLOBAL_WAIT(NUM_THREADS);
   // pthread_exit(NULL);
   return NULL;
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

  // Barrier initialization
  if(pthread_barrier_init(&barr, NULL, NUM_THREADS))
  {
    printf("Could not create a barrier\n");
    return -1;
  }

  pthread_t threads[NUM_THREADS];
  int rc;
  long t;
  
  for(t=0;t<NUM_THREADS;t++){
    printf("In main: creating thread %ld\n", t);
    rc = pthread_create(&threads[t], NULL, multicore_spmv, (void *)t);
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
