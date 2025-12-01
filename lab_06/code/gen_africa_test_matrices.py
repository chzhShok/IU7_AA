import numpy as np

from algorithms import brute_force_path, ant_algorithm_path


def load_africa_matrix() -> np.ndarray:
    return np.loadtxt("code/data/africa.csv", dtype=float)


def submatrix(matrix: np.ndarray, indices: list[int]) -> np.ndarray:
    idx = np.array(indices)
    return matrix[np.ix_(idx, idx)]


def print_matrix(name: str, mat: np.ndarray) -> None:
    print(f"\n{name} ({mat.shape[0]}x{mat.shape[0]}):")
    for row in mat.astype(int):
        print(" ".join(f"{val:4d}" for val in row))


def main() -> None:
    base = load_africa_matrix()

    # Порядок стран в africa.csv:
    countries = [
        "Algeria",    # 0
        "Libya",      # 1
        "Egypt",      # 2
        "Mali",       # 3
        "Niger",      # 4
        "Chad",       # 5
        "Sudan",      # 6
        "Nigeria",    # 7
        "Kenya",      # 8
        "Ethiopia",   # 9
        "Madagascar", # 10
    ]

    tests = [
        ("Тест 1: Северная Африка (6 стран)",
         [0, 1, 2, 3, 4, 5]),
        ("Тест 2: Центральная/Восточная Африка (6 стран)",
         [4, 5, 6, 7, 8, 9]),
        ("Тест 3: Полная карта (11 стран)",
         list(range(len(countries)))),
    ]

    for title, idx in tests:
        mat = submatrix(base, idx)
        print_matrix(title, mat)

        brute_len, brute_path = brute_force_path(mat)
        ant_len, ant_path = ant_algorithm_path(
            mat, alpha=0.5, beta=0.7, evaporation=0.5, days=150
        )

        human_brute = [i + 1 for i in brute_path]
        human_ant = [i + 1 for i in ant_path]

        print(f"{title}:")
        print(f"  Полный перебор: длина = {brute_len:.0f}, маршрут = {human_brute}")
        print(f"  Муравьиный алгоритм: длина = {ant_len:.0f}, маршрут = {human_ant}")


if __name__ == "__main__":
    main()


