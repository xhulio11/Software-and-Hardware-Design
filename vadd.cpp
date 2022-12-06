#include "header.h"

extern "C" {
	void vadd(const int *source_in1, const int *source_in2,
			int *source_result){

		int matrix_1[n][m];
		int matrix_2[m][p];
		int matrix_result[n][p];

#pragma HLS INTERFACE m_axi port = source_in1 offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = source_in2 offset = slave bundle = gmem
#pragma HLS INTERFACE m_axi port = source_result offset = slave bundle = gmem
#pragma HLS INTERFACE s_axilite port = source_in1 bundle = control
#pragma HLS INTERFACE s_axilite port = source_in2 bundle = control
#pragma HLS INTERFACE s_axilite port = source_result bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

		// Reading the first Matrix
		for(int i = 0; i < n; i++){
		#pragma HLS pipeline
			for(int j = 0; j < m; j++){
				matrix_1[i][j] = source_in1[i*m + j];
			}
		}

		// Reading the second Matrix
		for(int i = 0; i < m; i++){
		#pragma HLS pipeline
			for(int j = 0; j < p; j++){
				matrix_2[i][j] = source_in2[i*p + j];
			}
		}

		/*
		#pragma HLS array_partition variable = matrix_1 complete dim = 2
		#pragma HLS array_partition variable = matrix_2 complete dim = 1
	*/
		// Implementing the multiplication in hardware
		OUT:for(int i = 0; i < n; i++){

			MID:for(int j = 0; j < p; j++){
				#pragma HLS pipeline
				int sum = 0;

				LOW:for(int k = 0; k < m; k++){
					sum += matrix_1[i][k] * matrix_2[k][j];
				}
				matrix_result[i][j] = sum;
			}
		}

		// Writing the results in output
		for(int i = 0; i < n; i++){
		#pragma HLS pipeline
			for(int j = 0; j < p; j++){
				source_result[i*p + j] = matrix_result[i][j];
			}
		}
	}
}
