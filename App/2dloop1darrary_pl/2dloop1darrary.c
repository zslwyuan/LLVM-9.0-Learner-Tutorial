void f ( int *A) {
  int N = 100;
  int M = 50;
  for ( int j = 0; j < N; j++ )
    for ( int i = 0; i < M; i++ )
      A[i + 8] = A[i] + 1;
}
