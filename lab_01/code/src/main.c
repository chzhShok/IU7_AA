#include "classic_multiplication.h"
#include "io_matrix.h"
#include "measure_time.h"
#include "optimized_vinograd_multiplication.h"
#include "vinograd_multiplication.h"

#include <stdio.h>

static void menu(void) {
  printf("--------------- Menu --------------\n");
  printf("1. Классический алгоритм умножения матриц\n");
  printf("2. Алгоритм Винограда умножения матриц\n");
  printf("3. Оптимизированный алгоритм Винограда\n");
  printf("4. Замерить время (диапазон)\n");
  printf("0. Выход\n");
}

int main(void) {
  int choose = -1;
  while (choose != 0) {
    menu();
    printf("Введите номер пункта: ");
    if (scanf("%d", &choose) != 1)
      return 0;

    switch (choose) {
    case 1: {
      Matrix a = {0}, b = {0}, res = {0};
      int n1, m1, n2, m2;
      printf("Введите размеры первой матрицы (n m): ");
      if (scanf("%d %d", &n1, &m1) != 2)
        break;
      printf("Введите первую матрицу:\n");
      input_matrix(&a, n1, m1);

      printf("Введите размеры второй матрицы (n m): ");
      if (scanf("%d %d", &n2, &m2) != 2) {
        free_matrix(&a);
        break;
      }
      printf("Введите вторую матрицу:\n");
      input_matrix(&b, n2, m2);

      res = classic_multiplication(&a, &b);
      printf("Результат умножения:\n");
      print_matrix(&res);
      free_matrix(&a);
      free_matrix(&b);
      free_matrix(&res);
      break;
    }
    case 2: {
      Matrix a = {0}, b = {0}, res = {0};
      int n1, m1, n2, m2;
      printf("Введите размеры первой матрицы (n m): ");
      if (scanf("%d %d", &n1, &m1) != 2)
        break;
      printf("Введите первую матрицу:\n");
      input_matrix(&a, n1, m1);

      printf("Введите размеры второй матрицы (n m): ");
      if (scanf("%d %d", &n2, &m2) != 2) {
        free_matrix(&a);
        break;
      }
      printf("Введите вторую матрицу:\n");
      input_matrix(&b, n2, m2);

      res = vinograd_multiplication(&a, &b);
      printf("Результат умножения (Виноград):\n");
      print_matrix(&res);
      free_matrix(&a);
      free_matrix(&b);
      free_matrix(&res);
      break;
    }
    case 3: {
      Matrix a = {0}, b = {0}, res = {0};
      int n1, m1, n2, m2;
      printf("Введите размеры первой матрицы (n m): ");
      if (scanf("%d %d", &n1, &m1) != 2)
        break;
      printf("Введите первую матрицу:\n");
      input_matrix(&a, n1, m1);

      printf("Введите размеры второй матрицы (n m): ");
      if (scanf("%d %d", &n2, &m2) != 2) {
        free_matrix(&a);
        break;
      }
      printf("Введите вторую матрицу:\n");
      input_matrix(&b, n2, m2);

      res = optimized_vinograd_multiplication(&a, &b);
      printf("Результат умножения (Опт. Виноград):\n");
      print_matrix(&res);
      free_matrix(&a);
      free_matrix(&b);
      free_matrix(&res);
      break;
    }
    case 4: {
      int startN, endN, step, iterations;
      printf("Введите начальный размер, конечный и шаг: ");
      if (scanf("%d %d %d", &startN, &endN, &step) != 3)
        break;
      printf("Введите количество повторов: ");
      if (scanf("%d", &iterations) != 1)
        break;
      measure_algorithms_range(startN, endN, step, iterations);
      break;
    }
    case 0:
      printf("Выход из программы.\n");
      break;
    default:
      printf("Некорректный ввод. Пожалуйста, попробуйте снова.\n");
    }
  }
  return 0;
}
