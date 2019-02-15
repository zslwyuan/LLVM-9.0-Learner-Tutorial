#include<stdio.h>
#include <stdlib.h>

void f ( int *A) {
  int N = 100;
  int M = 50;
  int *aa = (int*)malloc(sizeof(int)*50);
  for ( int j = 0; j < N; j++ )
  {
    
    aa[j] = j;
    for ( int i = 0; i < M; i++ )
      A[i + 8] = A[i] + 1 + aa[i];
    
  }
  for ( int j = 0; j < N; j++ )
  {
    
    aa[j] = aa[j] + j;
    for ( int i = 0; i < M; i++ )
      A[i + 8] = A[i] + 1 + aa[i];
    
  }
  free(aa);
}
