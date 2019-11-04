
#include <math.h>
#define MINI_DATASET
#  ifdef MINI_DATASET
#   define TSTEPS 20
#   define N 30
#  endif

#define TSTEPS 20
#define DATA_TYPE float
/* Main computational kernel. The whole function will be timed,
   including the call and return. */
   
void kernel_jacobi_2d(int tsteps,
			    int n,
			    DATA_TYPE A[N][N],
			    DATA_TYPE B[N][N])
{
  int t, i, j;

      for (i = 1; i < N - 1; i++)
        for (j = 1; j < N - 1; j++)
          B[i][j] = (DATA_TYPE)(0.2) * (A[i][j] + A[i][j-1] + A[i][1+j] + A[1+i][j] + A[i-1][j]);
      for (i = 1; i < N - 1; i++)
        for (j = 1; j < N - 1; j++)
          A[i][j] = (DATA_TYPE)(0.2) * (B[i][j] + B[i][j-1] + B[i][1+j] + B[1+i][j] + B[i-1][j]);

}

