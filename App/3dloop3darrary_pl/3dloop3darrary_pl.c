void f ( int A[50][100][20]) {
  int N = 100;
  int M = 50;
  int R = 20;

  for ( int j = 1; j < N; j++ )
    for ( int i = 0; i < M; i++ )
      A[i][j][0] = A[i-1][j-1][0] + A[i][j-1][0] + A[i-1][j][0] + 1;
  for ( int j = 1; j < N; j++ )
    for ( int i = 0; i < M; i++ )
       for ( int k = 1; k < R; k++ )
      A[i][j][k] = A[i-1][j-1][k]-1 + A[i][j-1][k-1] + A[i-1][j][k-1] + 1;
  return;
}
