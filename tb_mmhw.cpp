#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tb_mmhw.h"

unsigned char matrix_1 [n][m];
unsigned char matrix_2 [m][p];
unsigned int matrix_result_sw [n][p];
unsigned int matrix_result_hw [n][p];

void mmhw(unsigned char matrix_1[n][m], unsigned char matrix_2[m][p],
		unsigned int matrix_result[n][p]);

void printer(int rows, int columns, unsigned int matrix[n][p]){

    for(int i = 0; i < rows; i++){
        for(int j = 0; j < columns; j++){
            printf("%d \t",matrix[i][j]);
        }
        printf("\n");
    }
}

void mmsw(unsigned char matrix_1[n][m], unsigned char matrix_2[m][p]){

    for(int i = 0; i < n; i++){
        for(int j = 0; j < p; j++){
            unsigned int sum = 0;
            for(int k = 0; k < m  ; k++){
                sum += matrix_1[i][k] * matrix_2[k][j];

            }
            matrix_result_sw[i][j] = sum;
        }
    }
}

bool tester(unsigned int matrix_result_sw[n][p],unsigned int matrix_result_hw[n][p]){
	for(int i = 0; i < n; i++){
		for(int j = 0; j < p; j++){
			if(matrix_result_sw[i][j] != matrix_result_hw[i][j] )return false;
		}
	}
	return true;
}



int main(){

    printf("\nMatrix 1: \n");
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            matrix_1[i][j] = (unsigned char)rand()%256;
            printf("%d\t",matrix_1[i][j]);
        }
        printf("\n");
    }

    printf("\nMatrix 2: \n");
    for(int i = 0; i < m; i++){
        for(int j = 0; j < p; j++){
            matrix_2[i][j] = (unsigned char)rand()%256;
            printf("%d\t",matrix_2[i][j]);
        }
        printf("\n");
    }

    mmhw(matrix_1,matrix_2, matrix_result_hw);
    mmsw(matrix_1,matrix_2);
    printf("\nSoftware Result is: \n");
    printer(n,p, matrix_result_sw);
    printf("\nHardware Result is: \n");
    printer(n,p, matrix_result_hw);
    printf("\n");

    if(tester(matrix_result_sw, matrix_result_hw)){
    	printf("--------------\nTest Passed!\n---------------\n");
    }
    else printf("--------------\nTest Failed!\n---------------\n");


}
