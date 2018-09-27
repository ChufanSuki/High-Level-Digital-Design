#include <stdio.h>

#define SIZE 128
#define N 	 10

void fir(int input[SIZE], int output[SIZE])
{
		int i, j;
		int coeff[N] = {13, -2, 9, 11, 26, 18, 95, -43, 6, 74};
		for(i = 0; i < N; ++i)
		{
				output[i] = 0;
				for(j = 0; j <= i; ++j)
						output[i] += coeff[j] * input[i-j];
		}
}

int main()
{
}
