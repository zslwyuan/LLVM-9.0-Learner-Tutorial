void f ( int A[50][100]) {
  int N = 100;
  int M = 50;
  for ( int j = 1; j < N; j++ )
  {
      for ( int i = 0; i < M; i++ )
      {
        if (i%2)
        {
          A[i][j] = A[i-1][j-1] + A[i][j-1] + A[i-1][j] + 2;    
        }
        else
        {
          A[i][j] = A[i-1][j-1] + 1;    
        }
        
      }
  }
}
