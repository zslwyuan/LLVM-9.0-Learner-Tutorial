
#include <math.h>
#define DATA_TYPE float
/* Include polybench common header. */

/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
#ifndef _CORRELATION_H
# define _CORRELATION_H


#define MINI_DATASET
# if !defined(M) && !defined(N)
/* Define sample dataset sizes. */
#  ifdef MINI_DATASET
#   define M 28
#   define N 32
#  endif

#  ifdef SMALL_DATASET
#   define M 80
#   define N 100
#  endif

#  ifdef MEDIUM_DATASET
#   define M 240
#   define N 260
#  endif

#  ifdef LARGE_DATASET
#   define M 1200
#   define N 1400
#  endif

#  ifdef EXTRALARGE_DATASET
#   define M 2600
#   define N 3000
#  endif


#endif /* !(M N) */


/* Default data type */
# if !defined(DATA_TYPE_IS_INT) && !defined(DATA_TYPE_IS_FLOAT) && !defined(DATA_TYPE_IS_DOUBLE)
#  define DATA_TYPE_IS_DOUBLE
# endif

#ifdef DATA_TYPE_IS_INT
#  define DATA_TYPE int
#  define DATA_PRINTF_MODIFIER "%d "
#endif

#ifdef DATA_TYPE_IS_FLOAT
#  define DATA_TYPE float
#  define DATA_PRINTF_MODIFIER "%0.2f "
#  define (x) x##f
#  define SQRT_FUN(x) sqrtf(x)
#  define EXP_FUN(x) expf(x)
#  define POW_FUN(x,y) powf(x,y)
# endif

#endif /* !_CORRELATION_H */


/* Main computational kernel. The whole function will be timed,
   including the call and return. */

void kernel_correlation(int m, int n,
			DATA_TYPE float_n,
			DATA_TYPE data[N][M],
			DATA_TYPE corr[M][M],
			DATA_TYPE mean[M],
			DATA_TYPE stddev[M])
{
  int i, j, k;

  DATA_TYPE eps = (0.1);


  for (j = 0; j < M; j++)
    {
      mean[j] = (0.0);
      for (i = 0; i < N; i++)
	mean[j] += data[i][j];
      mean[j] /= float_n;
    }


   for (j = 0; j < M; j++)
    {
      stddev[j] = (0.0);
      for (i = 0; i < N; i++)
        stddev[j] += (data[i][j] - mean[j]) * (data[i][j] - mean[j]);
      stddev[j] /= float_n;
      stddev[j] = sqrt(stddev[j]);
      /* The following in an inelegant but usual way to handle
         near-zero std. dev. values, which below would cause a zero-
         divide. */
      stddev[j] = stddev[j] <= eps ? (1.0) : stddev[j];
    }

  /* Center and reduce the column vectors. */
  for (i = 0; i < N; i++)
    for (j = 0; j < M; j++)
      {
        data[i][j] -= mean[j];
        data[i][j] /= sqrt(float_n) * stddev[j];
      }



}
