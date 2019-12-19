// void f ( int A[50][100]) {
//   int N = 100;
//   int M = 50;
//   for ( int j = 3; j < N; j++ )
//     for ( int i = 1; i < M; i++ )
//       A[i][j] = A[i-1][j-1] + A[i][j-1] + A[i-1][j] + A[i-1][j-2] + A[i-1][j-3] + 1;
//   return;
// }

void f(int A[50][100])
{
    int N = 100;
    int M = 50;
    for (int i = 1; i < M; i++)
        for (int j = 3; j < N; j++)
        {
            int a = A[i][j - 3];
            int b = A[i - 1][j - 1];
            int c = A[i][j - 2];
            A[i][j] = a + b + c;
        }
    return;
}