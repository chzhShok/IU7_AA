#include "optimized_vinograd_multiplication.h"
#include <stdio.h>
#include <stdlib.h>

Matrix optimized_vinograd_multiplication(const Matrix *A, const Matrix *B) {
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

  const int n = A->rows;
  const int m = A->cols;
  const int p = B->cols;

  const int halfM = m >> 1;
  const int hasOdd = (m & 1);

  int *MulH = (int *)calloc((size_t)n, sizeof(int));
  int *MulV = (int *)calloc((size_t)p, sizeof(int));

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < halfM; ++j) {
      MulH[i] += A->data[i][j << 1] * A->data[i][(j << 1) + 1];
    }
  }

  for (int i = 0; i < p; ++i) {
    for (int j = 0; j < halfM; ++j) {
      MulV[i] += B->data[j << 1][i] * B->data[(j << 1) + 1][i];
    }
  }

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < p; ++j) {
      C.data[i][j] = -MulH[i] - MulV[j];
      for (int k = 0; k < halfM; ++k) {
        C.data[i][j] += (A->data[i][k << 1] + B->data[(k << 1) + 1][j]) *
                        (A->data[i][(k << 1) + 1] + B->data[k << 1][j]);
      }
    }
  }

  if (hasOdd) {
    const int last = m - 1;
    for (int i = 0; i < n; ++i) {
      const int aLast = A->data[i][last];
      for (int j = 0; j < p; ++j) {
        C.data[i][j] += aLast * B->data[last][j];
      }
    }
  }

  free(MulH);
  free(MulV);

  return C;
}
