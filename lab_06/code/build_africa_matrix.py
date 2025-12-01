import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

OUT_MATRIX = ROOT / "code" / "data" / "africa.csv"


# Набор стран для графа (11 вершин, >= 10 по общему заданию)
# Порядок важен: индексы строк/столбцов в матрице будут соответствовать этому списку.
COUNTRIES = [
    "Algeria",
    "Libya",
    "Egypt",
    "Mali",
    "Niger",
    "Chad",
    "Sudan",
    "Nigeria",
    "Kenya",
    "Ethiopia",
    "Madagascar",
]

# Координаты стран (широта, долгота) взяты из all_countries.md
COORDS = {
    "Algeria": (28.033886, 1.659626),
    "Libya": (26.3351, 17.228331),
    "Egypt": (26.820553, 30.802498),
    "Mali": (17.570692, -3.996166),
    "Niger": (17.607789, 8.081666),
    "Chad": (15.454166, 18.732207),
    "Sudan": (12.862807, 30.217636),
    "Nigeria": (9.081999, 8.675277),
    "Kenya": (-0.023559, 37.906193),
    "Ethiopia": (9.145, 40.489673),
    "Madagascar": (-18.766947, 46.869107),
}


# Страны, считающиеся пустынными (движение по пустыне – медленнее)
DESERT_COUNTRIES = {
    "Algeria",
    "Libya",
    "Egypt",
    "Mali",
    "Niger",
    "Chad",
    "Sudan",
}

# Единственная страна, до которой добираемся по воде
WATER_COUNTRY = "Madagascar"


def haversine(lat1, lon1, lat2, lon2):
    """
    Большая окружность (км) между двумя точками по широте/долготе.
    """
    r = 6371.0
    phi1 = math.radians(lat1)
    phi2 = math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlambda = math.radians(lon2 - lon1)

    a = math.sin(dphi / 2) ** 2 + math.cos(phi1) * math.cos(phi2) * math.sin(
        dlambda / 2
    ) ** 2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
    return r * c


def terrain_factor(name_a: str, name_b: str) -> float:
    """
    Коэффициент времени с учётом местности:
      - два пустынных государства: *1.5 (дольше по пустыне)
      - одно из государств — Мадагаскар: *0.7 (быстрее по воде)
      - иначе: 1.0
    """
    if name_a == WATER_COUNTRY or name_b == WATER_COUNTRY:
        return 0.7

    if name_a in DESERT_COUNTRIES and name_b in DESERT_COUNTRIES:
        return 1.5

    return 1.0


def main():
    # Проверка, что для всех выбранных стран заданы координаты
    missing = [c for c in COUNTRIES if c not in COORDS]
    if missing:
        raise SystemExit(f"Не найдены координаты стран: {missing}")

    n = len(COUNTRIES)
    matrix = [[0.0 for _ in range(n)] for _ in range(n)]

    for i, name_i in enumerate(COUNTRIES):
        lat_i, lon_i = COORDS[name_i]
        for j, name_j in enumerate(COUNTRIES):
            if i == j:
                matrix[i][j] = 0.0
                continue
            lat_j, lon_j = COORDS[name_j]
            dist = haversine(lat_i, lon_i, lat_j, lon_j)
            k = terrain_factor(name_i, name_j)
            time_val = dist * k
            matrix[i][j] = time_val

    OUT_MATRIX.parent.mkdir(parents=True, exist_ok=True)
    with OUT_MATRIX.open("w", encoding="utf-8") as f:
        for i in range(n):
            row_ints = [str(int(round(x))) for x in matrix[i]]
            f.write(" ".join(row_ints) + "\n")

    print(f"Матрица на {n} стран записана в {OUT_MATRIX}")
    print("Порядок стран (индексы 0..n-1):")
    for idx, name in enumerate(COUNTRIES):
        print(f"{idx}: {name}")


if __name__ == "__main__":
    main()


