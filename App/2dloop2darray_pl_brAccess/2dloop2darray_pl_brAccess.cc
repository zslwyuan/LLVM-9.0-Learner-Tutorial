void f ( int A[50][100]) {
  int N = 100;
  int M = 50;
  int B[50][100];
  // for ( int j = 0; j < N; j++ )
  //   for ( int i = 0; i < M; i++ )
      
  for ( int j = 1; j < N; j++ )
    for ( int i = 1; i < M; i++ )
    {
      B[i][j]=i;
      A[i][j] = A[i-1][j] * A[i-1][j];//+A[i-2][j];
      //A[i][j] = i%2? A[i-1][j]:i;//+A[i-2][j];
    }
  // for ( int j = 1; j < N; j++ )
  //   for ( int i = 1; i < M; i++ )
  //   {
  //     B[i][j]=i;
  //     A[i][j] =  A[i-1][j]+A[i][j]+A[i-2][j];
  //   }
  return;
}
