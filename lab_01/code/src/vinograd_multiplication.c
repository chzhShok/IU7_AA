#include "vinograd_multiplication.h"
#include <stdio.h>
#include <stdlib.h>

Matrix vinograd_multiplication(const Matrix *A, const Matrix *B) {
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
  int m = A->cols;
  int p = B->cols;

  int *MulH = (int *)calloc((size_t)n, sizeof(int));
  int *MulV = (int *)calloc((size_t)p, sizeof(int));

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m / 2; ++j) {
      MulH[i] += A->data[i][2 * j] * A->data[i][2 * j + 1];
    }
  }

  for (int i = 0; i < p; ++i) {
    for (int j = 0; j < m / 2; ++j) {
      MulV[i] += B->data[2 * j][i] * B->data[2 * j + 1][i];
    }
  }

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < p; ++j) {
      C.data[i][j] = -MulH[i] - MulV[j];
      for (int k = 0; k < m / 2; ++k) {
        C.data[i][j] += (A->data[i][2 * k] + B->data[2 * k + 1][j]) *
                        (A->data[i][2 * k + 1] + B->data[2 * k][j]);
      }
    }
  }

  if (m % 2 == 1) {
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < p; ++j) {
        C.data[i][j] += A->data[i][m - 1] * B->data[m - 1][j];
      }
    }
  }

  free(MulH);
  free(MulV);

  return C;
}
