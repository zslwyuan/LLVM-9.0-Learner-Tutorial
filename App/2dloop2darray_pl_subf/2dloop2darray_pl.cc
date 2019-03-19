int subFunc1(int i, int j)
{
   i = i + 1;
   j = j * 2;
   for (int k = 0; k < i; k++)
      if (k%2)
        i+=k;
      else
        j+=k;
      
   return i+j;
}

void f ( int A[50][100]) {
  int N = 100;
  int M = 50;
  for ( int j = 1; j < N; j++ )
    for ( int i = 1; i < M; i++ )
    {
        A[i][j] = A[i-1][j-1] + A[i][j-1] + A[i-1][j] + subFunc1(i,j);
    }
      
  return;
}
