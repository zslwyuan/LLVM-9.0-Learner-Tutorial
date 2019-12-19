// void f1 ( int A[50][100]) {
//   int N = 100;
//   int M = 50;
//   int B[50][100];
//   for ( int j = 0; j < N; j++ )
//     for ( int i = 0; i < M; i++ )
//       B[i][j]=i;
//   for ( int j = 1; j < N; j++ )
//     for ( int i = 1; i < M; i++ )
//       A[i][j] = i%2? A[i-1][j]:B[i][j];
//   return;
// }

// void f3 ( int A[50][100]) {
//   int N = 100;
//   int M = 50;
//   int B[50][100];
//   for ( int j = 0; j < N; j++ )
//     for ( int i = 0; i < M; i++ )
//       B[i][j]=i;
//   for ( int j = 1; j < N; j++ )
//     for ( int i = 1; i < M; i++ )
//       A[i][j] = i%2? A[i-1][j]:B[i][j];
//   return;
// }

void f3(int A[50][100])
{
    int N = 100;
    int M = 50;
    // int B[50][100];
    // for ( int j = 0; j < N; j++ )
    //   for ( int i = 0; i < M; i++ )
    //     B[i][j]=i;
    for (int j = 1; j < N; j++)
        for (int i = 1; i < M; i++)
        {
            switch (i % 3)
            {
            case 0:
                A[i][j] = i;
                break;
            case 1:
                A[i][j] = A[i - 1][j];
                break;
            case 2:
                A[i][j] = A[i - 1][j - 1];
            default:
                break;
            }
        }
    return;
}
