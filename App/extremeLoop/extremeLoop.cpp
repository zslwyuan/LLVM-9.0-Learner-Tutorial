

void extremeLoop(int A[6], int B[8])
{
  int i, j, k;
  for (i=1;i<6;i++)
    A[i] = A[i-1] + 1;
  for (i=1;i<8;i++)
    B[i] = B[i-1] * 13;
}
