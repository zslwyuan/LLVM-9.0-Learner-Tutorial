void f ( int A[5000]) {
  int N = 100;
  int M = 50;
  for ( int j = 1; j < N; j++ )
    for ( int i = 0; i < M; i++ )
      A[i*100+j] = A[(i-1)*100+j-1] + A[i*100+j-1] + A[(i-1)*100+j] + 1;
  for ( int j = 1; j < N; j++ )
    for ( int i = 0; i < M; i++ )
      A[i*100+j] = A[(i-1)*100+j-1] + A[i*100+j-1] + A[(i-1)*100+j] + 1;
  return;
}
