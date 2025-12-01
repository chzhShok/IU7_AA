from typing import List, Tuple

import numpy as np
import os

from algorithms import brute_force_path, ant_algorithm_path
from constants import DEFAULT_DATA_FILE


Matrix = np.ndarray


def generate_matrix(size: int, rand_start: int, rand_end: int) -> Matrix:
    matrix = np.zeros((size, size), dtype=int)

    for i in range(size):
        for j in range(i + 1, size):
            w = np.random.randint(rand_start, rand_end + 1)
            matrix[i, j] = w
            matrix[j, i] = w

    return matrix


def generate_matrix_file(
    filename: str,
    size: int,
    rand_start: int,
    rand_end: int,
) -> str:
    matrix = generate_matrix(size, rand_start, rand_end)
    path = "data/" + filename

    with open(path, "w", encoding="utf-8") as f:
        for i in range(size):
            row = " ".join(str(int(x)) for x in matrix[i])
            f.write(row + "\n")

    return f"Файл {filename} успешно сгенерирован\n"


def read_file_matrix(filename: str) -> Matrix:
    path = "data/" + filename
    with open(path, "r", encoding="utf-8") as f:
        rows = [list(map(int, line.split())) for line in f if line.strip()]

    size = len(rows)
    matrix = np.zeros((size, size), dtype=int)
    for i, row in enumerate(rows):
        for j, value in enumerate(row):
            matrix[i, j] = value

    return matrix


def list_data_files() -> List[str]:
    data_dir = "data"
    if not os.path.isdir(data_dir):
        print("\nКаталог data/ отсутствует, он будет создан.")
        os.makedirs(data_dir, exist_ok=True)
        return []

    files = sorted(
        f
        for f in os.listdir(data_dir)
        if os.path.isfile(os.path.join(data_dir, f))
    )

    print(f"\nДоступные файлы (каталог data/), всего {len(files)}:")
    for i, name in enumerate(files, start=1):
        print(f"{i}. {name}")

    return files


def update_file() -> None:
    try:
        option = int(input("\nДобавить НОВЫЙ файл? (1 - да, 2 - перезаписать существующий): "))
    except Exception:
        print("Неверный ввод пункта меню.")
        return

    if option == 1:
        filename = input("Введите имя нового файла (например, my_graph.csv): ")
    elif option == 2:
        files = list_data_files()
        if not files:
            print("Нет существующих файлов, создаём новый.")
            filename = input("Введите имя нового файла (например, my_graph.csv): ")
        else:
            try:
                idx = int(input("Выберите файл для перезаписи: ")) - 1
                filename = files[idx]
            except Exception:
                print("Неверно выбран файл.")
                return
    else:
        print("Неверный пункт.")
        return

    try:
        size = int(input("Введите размер матрицы (число вершин графа): "))
        rand_start = int(input("Минимальное значение веса ребра: "))
        rand_end = int(input("Максимальное значение веса ребра: "))
    except Exception:
        print("Неверный ввод параметров генерации.")
        return

    print(generate_matrix_file(filename, size, rand_start, rand_end))


def print_matrix() -> None:
    files = list_data_files()
    if not files:
        print("\nНет файлов с матрицами.")
        return

    try:
        idx = int(input("Выберите файл: ")) - 1
        filename = files[idx]
    except Exception:
        print("Неверно выбран файл.")
        return

    matrix = read_file_matrix(filename)

    print(f"\nМатрица из файла {filename}:\n")
    for i in range(matrix.shape[0]):
        for j in range(matrix.shape[1]):
            print(f"{int(matrix[i, j]):4d}", end="")
        print()


def read_matrix() -> Matrix:
    data_dir = "data"
    os.makedirs(data_dir, exist_ok=True)

    default_path = os.path.join(data_dir, DEFAULT_DATA_FILE)
    if os.path.isfile(default_path):
        return read_file_matrix(DEFAULT_DATA_FILE)

    files = list_data_files()
    if not files:
        raise RuntimeError(
            "Нет входных файлов с матрицами. "
            "Сгенерируйте их через пункт меню 'Обновить / сгенерировать матрицу'."
        )

    try:
        idx = int(input("Выберите файл: ")) - 1
        filename = files[idx]
    except Exception:
        raise RuntimeError("Неверно выбран файл с матрицей.")

    return read_file_matrix(filename)


def parse_brute_force() -> None:
    matrix = read_matrix()
    best_len, best_path = brute_force_path(matrix)

    human_path = [v + 1 for v in best_path]

    print("\nАлгоритм полного перебора (незамкнутый маршрут):")
    print(f"  Минимальная длина пути: {best_len}")
    print(f"  Маршрут (города, начиная с 1): {human_path}")


def _read_ants_params() -> Tuple[float, float, float, int]:
    try:
        alpha = float(input("\nВведите коэффициент alpha (вес феромона): "))
        beta = float(input("Введите коэффициент beta (вес видимости): "))
        evaporation = float(input("Введите коэффициент испарения (0..1): "))
        days = int(input("Введите количество итераций (дней): "))
    except Exception:
        print("Неверный ввод, используются значения по умолчанию.")
        alpha = 0.5
        beta = 0.5
        evaporation = 0.5
        days = 50

    return alpha, beta, evaporation, days


def parse_ant() -> None:
    matrix = read_matrix()
    alpha, beta, evaporation, days = _read_ants_params()

    best_len, best_path = ant_algorithm_path(
        matrix,
        alpha=alpha,
        beta=beta,
        evaporation=evaporation,
        days=days,
    )

    human_path = [v + 1 for v in best_path]

    print("\nМуравьиный алгоритм (незамкнутый маршрут):")
    print(f"  Минимальная длина пути: {best_len}")
    print(f"  Маршрут (города, начиная с 1): {human_path}")


def parse_all() -> None:
    matrix = read_matrix()
    alpha, beta, evaporation, days = _read_ants_params()

    brute_len, brute_path = brute_force_path(matrix)
    ant_len, ant_path = ant_algorithm_path(
        matrix,
        alpha=alpha,
        beta=beta,
        evaporation=evaporation,
        days=days,
    )

    brute_human = [v + 1 for v in brute_path]
    ant_human = [v + 1 for v in ant_path]

    print("\nАлгоритм полного перебора (незамкнутый маршрут):")
    print(f"  Минимальная длина пути: {brute_len}")
    print(f"  Маршрут: {brute_human}")

    print("\nМуравьиный алгоритм (незамкнутый маршрут):")
    print(f"  Минимальная длина пути: {ant_len}")
    print(f"  Маршрут: {ant_human}")


