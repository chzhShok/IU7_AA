import os

import matplotlib.pyplot as plt
import pandas as pd


def load_data(csv_path: str) -> pd.DataFrame:
    if not os.path.exists(csv_path):
        raise FileNotFoundError(f"CSV-файл не найден: {csv_path}")

    df = pd.read_csv(csv_path)
    required_cols = {"threads", "N", "avg_time_ms"}
    if not required_cols.issubset(df.columns):
        raise ValueError(f"Ожидались колонки {required_cols}, но в файле есть: {set(df.columns)}")

    return df


def plot_time_comparison(df: pd.DataFrame, output_path: str | None = None) -> None:
    """
    Строит один график с двумя линиями:
    - линейный алгоритм (threads == 0)
    - конвейерный алгоритм (threads == 1)
    """
    # Сортируем по N, чтобы линии были аккуратными
    df_sorted = df.sort_values(by="N")

    linear = df_sorted[df_sorted["threads"] == 0]
    pipeline = df_sorted[df_sorted["threads"] == 1]

    if linear.empty or pipeline.empty:
        raise ValueError("В данных должны быть строки для threads == 0 и threads == 1.")

    plt.figure(figsize=(8, 5))

    # Переводим миллисекунды в секунды
    linear_time_s = linear["avg_time_ms"] / 1000.0
    pipeline_time_s = pipeline["avg_time_ms"] / 1000.0

    # Линейный алгоритм: синий цвет, круглые маркеры
    plt.plot(
        linear["N"],
        linear_time_s,
        label="Линейный",
        color="tab:blue",
        marker="o",
        linestyle="-",
        linewidth=2,
        markersize=6,
    )

    # Конвейерный алгоритм: оранжевый цвет, квадратные маркеры
    plt.plot(
        pipeline["N"],
        pipeline_time_s,
        label="Конвейерный",
        color="tab:orange",
        marker="s",
        linestyle="-",
        linewidth=2,
        markersize=6,
    )

    plt.xlabel("N (количество заявок)")
    plt.ylabel("Среднее время выполнения, с")
    plt.title("Сравнение времени выполнения линейного и конвейерного алгоритмов")
    plt.grid(True, alpha=0.3)
    plt.xticks([25, 50, 75, 100, 125])
    plt.legend()

    plt.tight_layout()

    if output_path is not None:
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        plt.savefig(output_path, dpi=300, bbox_inches="tight")
    else:
        plt.show()

    plt.close()


def main() -> None:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    csv_path = os.path.join(script_dir, "../result/experiments_time_avg.csv")

    # По умолчанию сохраняем картинку в images отчёта
    output_path = os.path.join(script_dir, "../report/images/experiments_time_avg.png")

    df = load_data(csv_path)
    plot_time_comparison(df, output_path=output_path)

    print(f"График сохранён в файл: {os.path.abspath(output_path)}")


if __name__ == "__main__":
    main()


