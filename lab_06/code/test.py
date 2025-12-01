from typing import List

from time import process_time

import matplotlib.pyplot as plt
import numpy as np

from algorithms import brute_force_path, ant_algorithm_path
from utils import generate_matrix


def test_time() -> None:
    try:
        size_start = int(input("\nВведите начальный размер матрицы: "))
        size_end = int(input("Введите конечный размер матрицы: "))
        if size_start <= 1 or size_end < size_start:
            raise ValueError
    except Exception:
        print("Неверно заданы размеры матрицы.")
        return

    sizes: List[int] = list(range(size_start, size_end + 1))
    time_brute: List[float] = []
    time_ant: List[float] = []

    print()
    for idx, n in enumerate(sizes, start=1):
        matrix = generate_matrix(n, 1, 20)

        start = process_time()
        brute_force_path(matrix)
        end = process_time()
        time_brute.append(end - start)

        start = process_time()
        ant_algorithm_path(matrix, alpha=0.5, beta=0.5, evaporation=0.5, days=70)
        end = process_time()
        time_ant.append(end - start)

        progress = 100.0 * idx / len(sizes)
        print(f"Прогресс: {progress:5.1f}%")

    print("\nРазмер | Время полного перебора | Время муравьиного алгоритма")
    print("-" * (7 + 1 + 25 + 1 + 29))
    for n, t_b, t_a in zip(sizes, time_brute, time_ant):
        print(f"{n:6d} | {t_b:23.6f} | {t_a:27.6f}")

    with open("table.txt", "w", encoding="utf-8") as f:
        for n, t_b, t_a in zip(sizes, time_brute, time_ant):
            f.write(f"{n:4d} & {t_b:10.6f} & {t_a:10.6f} \\\\ \\hline\n")

    fig = plt.figure(figsize=(10, 7))
    ax = fig.add_subplot()
    ax.plot(sizes, time_brute, label="Полный перебор")
    ax.plot(sizes, time_ant, label="Муравьиный алгоритм")
    plt.legend()
    plt.grid()
    plt.title("Временные характеристики алгоритмов\n(незамкнутый маршрут)")
    plt.xlabel("Размер матрицы (число городов)")
    plt.ylabel("Время (с)")
    plt.show()

 
def parametrization() -> None:
    size = 9
    matrices = [generate_matrix(size, 5, 50) for _ in range(3)]
    optimals = [brute_force_path(m)[0] for m in matrices]
    optimal_avg = sum(optimals) / len(optimals)

    alpha_vals = [0.1, 0.25, 0.5, 0.9]
    evaporation_vals = [0.1, 0.25, 0.5, 0.9]
    days_vals = [100, 300, 500]

    fname = "parametrization_class1.txt"
    f = open(fname, "w", encoding="utf-8")

    total = len(alpha_vals) * len(evaporation_vals)
    done = 0
    print()

    for alpha in alpha_vals:
        beta = 1.0 - alpha
        for rho in evaporation_vals:
            done += 1
            for days in days_vals:
                diffs = []
                for m, opt_len in zip(matrices, optimals):
                    res_len, _ = ant_algorithm_path(
                        m,
                        alpha=alpha,
                        beta=beta,
                        evaporation=rho,
                        days=days,
                    )
                    diffs.append(res_len - opt_len)

                diff_avg = sum(diffs) / len(diffs)

                sep = ", "
                ender = ""

                line = (
                    f"{alpha:4.1f}{sep}"
                    f"{beta:4.1f}{sep}"
                    f"{rho:4.1f}{sep}"
                    f"{days:4d}{sep}"
                    f"{int(optimal_avg):5d}{sep}"
                    f"{int(diff_avg):5d}{ender}\n"
                )
                f.write(line)

            progress = 100.0 * done / total
            print(f"Прогресс параметризации: {progress:5.1f}%")

    f.close()
    print(f"\nРезультаты параметризации записаны в файл {fname}")
