
void convs(int input[8][8], int output[8][8], int filter2[3][3], int filter1[5][5],
           int filter0[5][5])
{
    int mid0[8][8];
    for (int i = 2; i < 6; i++)
    {
        for (int j = 2; j < 6; j++)
        {
            int tmp = 0;
            for (int k = -2; k <= 2; k++)
            {
                for (int l = -2; l <= 2; l++)
                {
                    tmp += filter0[2 + k][2 + l] * input[i + k][j + l] * 123;
                }
            }
            mid0[i][j] = tmp > 0 ? tmp * 23 : tmp;
        }
    }

    int mid1[8][8];
    for (int i = 2; i < 6; i++)
    {
        for (int j = 2; j < 6; j++)
        {
            int tmp = 0;
            for (int k = -2; k <= 2; k++)
            {
                for (int l = -2; l <= 2; l++)
                {
                    tmp += filter1[2 + k][2 + l] * mid0[i + k][j + l] * 321;
                }
            }
            mid1[i][j] = tmp > 0 ? tmp * 23 : tmp;
        }
    }

    for (int i = 1; i < 7; i += 1)
    {
        for (int j = 1; j < 7; j += 1)
        {
            int tmp = 0;
            for (int k = -1; k <= 1; k++)
            {
                for (int l = -1; l <= 1; l++)
                {
                    tmp += filter2[1 + k][1 + l] * mid1[i + k][j + l];
                }
            }
            output[i][j] = tmp;
        }
    }
}