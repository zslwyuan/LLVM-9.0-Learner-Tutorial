typedef struct
{
    int a;
    char b;
    double c; 
} Simple2;

void f ( Simple2 A[50][100]) {
  int N = 100;
  int M = 50;
  for ( int j = 1; j < N; j++ )
    for ( int i = 1; i < M; i++ )
      A[i][j].a = A[i-1][j-1].b + A[i][j-1].b + A[i-1][j].b + 1;
  // for ( int j = 1; j < N; j++ )
  //   for ( int i = 1; i < M; i++ )
  //     A[i][j].c = A[i-1][j-1].a + A[i][j-1].a + A[i-1][j].a + 1.0/3.0;
  for ( int j = 1; j < N; j++ )
    for ( int i = 1; i < M; i++ )
      A[i][j].a = A[i-1][j-1].a + A[i][j-1].a + A[i-1][j].a + 1;
  return;
}
