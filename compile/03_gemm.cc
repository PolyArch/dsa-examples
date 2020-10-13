/*
Implementation based on algorithm described in:
The cache performance and optimizations of blocked algorithms
M. D. Lam, E. E. Rothberg, and M. E. Wolf
ASPLOS 1991
*/

#include "sim_timing.h"

#define TYPE int64_t
#undef N

//Algorithm Parameters
#define row_size 64
#define col_size 64
#define N row_size*col_size
#define block_size 8
#define NUMOFBLOCKS N/block_size/block_size

#ifndef U
#define U 8
#endif

void bbgemm(TYPE m1[N], TYPE m2[N], TYPE prod[N]){
  #pragma ss config
  {
    int i, k, j, jj, kk;
    int i_row, k_row;
    TYPE temp_x, mul;

    for (jj = 0; jj < row_size; jj += block_size){
        //for (kk = 0; kk < row_size; kk += block_size){
            for ( i = 0; i < row_size; ++i){
                #pragma ss stream nonblock
                for (k = 0; k < row_size; ++k){
                    i_row = i * row_size;
                    //k_row = (k  + kk) * row_size;
                    kk = 0;
                    temp_x = m1[i_row + k + kk];
                    #pragma ss dfg dedicated unroll(U)
                    for (j = 0; j < block_size; ++j){
                        mul = temp_x * m2[k_row + j + jj];
                        prod[i_row + j + jj] += mul;
                    }
                }
            }
        //}
    }
  }
}

TYPE a[N], b[N], c[N];

int main() {
  bbgemm(a, b, c);
  begin_roi();
  bbgemm(a, b, c);
  end_roi();
  sb_stats();
  return 0;
}
