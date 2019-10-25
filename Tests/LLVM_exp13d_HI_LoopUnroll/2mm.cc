
#define NI 16
#define NJ 18
#define NK 22
#define NL 24


void kernel_2mm(int ni, int nj, int nk, int nl,
		int alpha,
		int beta,
		int tmp[NI][NJ],
		int A[NI][NK],
		int B[NK][NJ],
		int C[NJ][NL],
		int D[NI][NL])
{
  int i, j, k;

  /* D := alpha*A*B*C + beta*D */
  for (i = 0; i < NI; i++)
    for (j = 0; j < NJ; j++)
    {
				tmp[i][j] = 0;
				loop1: for (k = 0; k < NK; ++k)
	  			tmp[i][j] += alpha * A[i][k] * B[k][j];
    }
  	for (i = 0; i < NI; i++)
    	for (j = 0; j < NL; j++)
      {
				D[i][j] *= beta;
					for (k = 0; k < NJ; ++k)
	  				D[i][j] += tmp[i][k] * C[k][j];
      }
}
