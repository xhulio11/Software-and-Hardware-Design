#include "tb_mmhw.h"

void mmhw(unsigned char matrix_1[n][m], unsigned char matrix_2[m][p],
		unsigned int matrix_result[n][p]){

	#pragma HLS array_partition variable = matrix_1 complete dim = 2
	#pragma HLS array_partition variable = matrix_2 complete dim = 1

	OUT:for(int i = 0; i < n; i++){

		MID:for(int j = 0; j < p; j++){
			#pragma HLS pipeline
			unsigned int sum = 0;

			LOW:for(int k = 0; k < m; k++){

				sum += matrix_1[i][k] * matrix_2[k][j];
			}
			matrix_result[i][j] = sum;
		}
	}
}
