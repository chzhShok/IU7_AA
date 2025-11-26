#pragma once

#include <stddef.h>
#include <stdlib.h>

typedef struct {
  int **data;
  int rows;
  int cols;
} Matrix;

static inline void create_matrix(Matrix *matrix, int rows, int cols) {
  matrix->rows = rows;
  matrix->cols = cols;
  if (rows > 0 && cols > 0) {
    matrix->data = (int **)malloc(sizeof(int *) * (size_t)rows);
    for (int i = 0; i < rows; ++i) {
      matrix->data[i] = (int *)malloc(sizeof(int) * (size_t)cols);
    }
  } else {
    matrix->data = NULL;
  }
}

static inline void free_matrix(Matrix *matrix) {
  if (matrix->data != NULL) {
    for (int i = 0; i < matrix->rows; ++i) {
      free(matrix->data[i]);
    }
    free(matrix->data);
  }
  matrix->data = NULL;
  matrix->rows = 0;
  matrix->cols = 0;
}
