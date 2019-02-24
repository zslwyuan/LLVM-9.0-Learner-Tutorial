void f ( int A[70][50][30]) {
  int N = 70;
  int M = 50;
  int R = 30;

  for ( int j = 1; j < M; j++ )
    for ( int i = 1; i < N; i++ )
      A[i][j][0] = A[i-1][j-1][0] + A[i][j-1][0] + A[i-1][j][0] + 1;
  for ( int j = 1; j < M; j++ )
    for ( int i = 1; i < N; i++ )
       for ( int k = 1; k < R; k++ )
      		A[i][j][k] = A[i-1][j-1][k]-1 + A[i][j-1][k-1] + A[i-1][j][k-1] + 1;
  return;
}
