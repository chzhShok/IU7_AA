#include "classic_multiplication.h"
#include <stdio.h>

Matrix classic_multiplication(const Matrix *A, const Matrix *B) {
  Matrix C = {0};

  if (A->cols != B->rows) {
    printf("Error: matrix dimensions mismatch (%d != %d)\n", A->cols, B->rows);
    return C;
  }

  if (A->rows <= 0 || A->cols <= 0 || B->rows <= 0 || B->cols <= 0) {
    printf("Error: matrix dimensions must be positive\n");
    return C;
  }

  create_matrix(&C, A->rows, B->cols);

  int n = A->rows;
  int m = B->cols;
  int p = A->cols;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      for (int k = 0; k < p; ++k) {
        C.data[i][j] += A->data[i][k] * B->data[k][j];
      }
    }
  }

  return C;
}
