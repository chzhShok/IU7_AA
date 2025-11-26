#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "classic_multiplication.h"
#include "matrix.h"
#include "optimized_vinograd_multiplication.h"
#include "vinograd_multiplication.h"

static void fill_random(Matrix *matrix, int rows, int cols, int minVal,
                        int maxVal) {
  create_matrix(matrix, rows, cols);
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      int v = minVal + rand() % (maxVal - minVal + 1);
      matrix->data[i][j] = v;
    }
  }
}

static uint64_t now_us(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000ull + (uint64_t)ts.tv_nsec / 1000ull;
}

static void print_table_header(void) {
  printf("\n");
  printf("+--------+------------------------+-------------------+--------------"
         "--------+\n");
  printf("|   N    |    Классический (мкс)  |    Виноград (мкс) |  Опт. "
         "Виноград (мкс) |\n");
  printf("+--------+------------------------+-------------------+--------------"
         "--------+\n");
}

static void print_table_row(int n, long long tStd, long long tVino,
                            long long tOpt, int iterations) {
  long long a = iterations > 0 ? tStd / iterations : tStd;
  long long b = iterations > 0 ? tVino / iterations : tVino;
  long long c = iterations > 0 ? tOpt / iterations : tOpt;
  printf("| %6d | %22lld | %17lld | %20lld |\n", n, a, b, c);
}

static void print_table_footer(void) {
  printf("+--------+------------------------+-------------------+--------------"
         "--------+\n");
}

void measure_algorithms_range(int startSize, int endSize, int step,
                              int iterations) {
  if (startSize <= 0 || endSize <= 0 || step <= 0 || startSize > endSize)
    return;

  srand(0);

  print_table_header();

  for (int n = startSize; n <= endSize; n += step) {
    int m = n, k = n;
    Matrix A = {0}, B = {0};
    fill_random(&A, n, m, 0, 100);
    fill_random(&B, m, k, 0, 100);

    long long tStd = 0, tVino = 0, tOpt = 0;
    for (int it = 0; it < iterations; ++it) {
      Matrix C1 = {0};
      uint64_t t0 = now_us();
      C1 = classic_multiplication(&A, &B);
      uint64_t t1 = now_us();
      tStd += (long long)(t1 - t0);
      free_matrix(&C1);

      Matrix C2 = {0};
      t0 = now_us();
      C2 = vinograd_multiplication(&A, &B);
      t1 = now_us();
      tVino += (long long)(t1 - t0);
      free_matrix(&C2);

      Matrix C3 = {0};
      t0 = now_us();
      C3 = optimized_vinograd_multiplication(&A, &B);
      t1 = now_us();
      tOpt += (long long)(t1 - t0);
      free_matrix(&C3);
    }

    print_table_row(n, tStd, tVino, tOpt, iterations);
    free_matrix(&A);
    free_matrix(&B);
  }

  print_table_footer();
  printf("\n");
}
