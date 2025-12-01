import itertools as it
from random import random
from math import inf
from typing import List, Sequence, Tuple

import numpy as np

from constants import MIN_PHEROMONE, EPS_PROBABILITY


Matrix = np.ndarray
Path = List[int]


def brute_force_path(matrix: Matrix) -> Tuple[float, Path]:
    n = matrix.shape[0]
    indices = list(range(n))

    best_len = inf
    best_path: Path = []

    for perm in it.permutations(indices):
        length = 0.0
        for i in range(n - 1):
            length += float(matrix[perm[i], perm[i + 1]])

        if length < best_len:
            best_len = length
            best_path = list(perm)

    return best_len, best_path


def _average_edge_weight(matrix: Matrix) -> float:
    n = matrix.shape[0]
    s = 0.0
    cnt = 0
    for i in range(n):
        for j in range(n):
            if i != j:
                s += float(matrix[i, j])
                cnt += 1
    return s / cnt if cnt else 1.0


def _init_pheromones(n: int, initial: float = 1.0) -> List[List[float]]:
    return [[initial for _ in range(n)] for _ in range(n)]


def _init_visibility(matrix: Matrix) -> List[List[float]]:
    n = matrix.shape[0]
    visibility: List[List[float]] = []
    for i in range(n):
        row: List[float] = []
        for j in range(n):
            if i == j:
                row.append(0.0)
            else:
                w = float(matrix[i, j])
                row.append(1.0 / w if w > 0 else 0.0)
        visibility.append(row)
    return visibility


def _calc_path_length(matrix: Matrix, route: Sequence[int]) -> float:
    length = 0.0
    for i in range(1, len(route)):
        length += float(matrix[route[i - 1], route[i]])
    return length


def _edge_in_path(i: int, j: int, route: Sequence[int]) -> bool:
    for k in range(1, len(route)):
        a, b = route[k - 1], route[k]
        if (a == i and b == j) or (a == j and b == i):
            return True
    return False


def _transition_probabilities(
    pheromones: List[List[float]],
    visibility: List[List[float]],
    visited: List[Path],
    ant_index: int,
    alpha: float,
    beta: float,
) -> List[float]:
    n = len(pheromones)
    probs = [0.0] * n

    current = visited[ant_index][-1]
    unvisited = [city for city in range(n) if city not in visited[ant_index]]

    for city in unvisited:
        tau = pheromones[current][city]
        eta = visibility[current][city]
        probs[city] = (tau ** alpha) * (eta ** beta)

    total = sum(probs)

    if total <= 0.0:
        if not unvisited:
            return probs
        uniform_p = 1.0 / float(len(unvisited))
        for city in unvisited:
            probs[city] = uniform_p
        return probs

    for i in range(n):
        if probs[i] > 0.0:
            probs[i] = max(probs[i] / total, EPS_PROBABILITY)

    total = sum(probs)
    if total > 0:
        probs = [p / total for p in probs]

    return probs


def _choose_next_city(probs: Sequence[float]) -> int:
    r = random()
    cumulative = 0.0

    for idx, p in enumerate(probs):
        cumulative += p
        if r <= cumulative:
            return idx

    for idx in range(len(probs) - 1, -1, -1):
        if probs[idx] > 0:
            return idx

    return 0


def _update_pheromones(
    matrix: Matrix,
    paths: List[Path],
    pheromones: List[List[float]],
    q: float,
    evaporation: float,
) -> List[List[float]]:
    n = len(pheromones)

    for i in range(n):
        for j in range(n):
            delta = 0.0
            for route in paths:
                if _edge_in_path(i, j, route):
                    length = _calc_path_length(matrix, route)
                    if length > 0:
                        delta += q / length

            pheromones[i][j] *= (1.0 - evaporation)
            pheromones[i][j] += delta

            if pheromones[i][j] < MIN_PHEROMONE:
                pheromones[i][j] = MIN_PHEROMONE

    return pheromones


def ant_algorithm_path(
    matrix: Matrix,
    alpha: float,
    beta: float,
    evaporation: float,
    days: int,
) -> Tuple[float, Path]:
    n = matrix.shape[0]
    ants = n

    q = _average_edge_weight(matrix)
    pheromones = _init_pheromones(n)
    visibility = _init_visibility(matrix)

    best_length = inf
    best_path: Path = []

    for _ in range(days):
        starts = list(range(n))
        visited: List[Path] = [[start] for start in starts]

        for ant_index in range(ants):
            while len(visited[ant_index]) < n:
                probs = _transition_probabilities(
                    pheromones,
                    visibility,
                    visited,
                    ant_index,
                    alpha,
                    beta,
                )
                next_city = _choose_next_city(probs)
                if next_city in visited[ant_index]:
                    unvisited = [
                        c
                        for c in range(n)
                        if c not in visited[ant_index]
                    ]
                    if not unvisited:
                        break
                    next_city = unvisited[0]

                visited[ant_index].append(next_city)

            length = _calc_path_length(matrix, visited[ant_index])
            if length < best_length:
                best_length = length
                best_path = visited[ant_index].copy()

        pheromones = _update_pheromones(
            matrix=matrix,
            paths=visited,
            pheromones=pheromones,
            q=q,
            evaporation=evaporation,
        )

    return best_length, best_path
