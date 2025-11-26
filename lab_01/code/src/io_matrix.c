#include "io_matrix.h"
#include <stdio.h>

void input_matrix(Matrix *matrix, int rows, int cols) {
  create_matrix(matrix, rows, cols);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      int v;
      if (scanf("%d", &v) != 1)
        v = 0;
      matrix->data[i][j] = v;
    }
  }
}

void print_matrix(const Matrix *matrix) {
  for (int i = 0; i < matrix->rows; ++i) {
    for (int j = 0; j < matrix->cols; ++j) {
      printf("%d ", matrix->data[i][j]);
    }
    printf("\n");
  }
}
