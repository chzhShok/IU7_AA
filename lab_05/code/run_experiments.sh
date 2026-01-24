#!/bin/bash

# Скрипт постановки эксперимента по измерению времени работы
# последовательной и параллельной (конвейерной) реализаций.
#
# Для каждого размера графа измеряется:
#  - время последовательного алгоритма (lab05_sequential, колонка 0 потоков);
#  - время конвейерной обработки (lab05_pipeline) при \textbf{фиксированном}
#    количестве потоков, равном DEFAULT_THREADS в Config.h.
# Каждый запуск повторяется несколько раз, результаты пишутся в CSV.

set -euo pipefail

cd "$(dirname "$0")"

BUILD_DIR="./build"
RESULT_DIR="../result"
mkdir -p "${RESULT_DIR}"

SIZES=(500 800 1000 1500 2000)
REPEATS=1
N_VALUES=(25 50 75 100 125)

OUT_CSV="${RESULT_DIR}/experiments_time.csv"
echo "vertices,threads,N,run,time_us" > "${OUT_CSV}"

# Убедимся, что проект собран
./build.sh >/dev/null

# echo "== Измерение последовательной версии (0 потоков) ==" >&2
# for n in "${SIZES[@]}"; do
#   graph="../graphs/graph_${n}.dot"
#   targets=$(cat "../graphs/graph_${n}_targets.txt")

#   for N_REQ in "${N_VALUES[@]}"; do
#     echo "  -> vertices=${n}, N=${N_REQ} (последовательный)" >&2
#     for r in $(seq 1 "${REPEATS}"); do
#       line=$("${BUILD_DIR}/lab05_sequential" "${graph}" 0 "${targets}" "${N_REQ}" | \
#              grep "Общее время последовательной обработки" || true)

#       if [[ -z "${line}" ]]; then
#         echo "Не удалось извлечь время для последовательной версии, n=${n}, N=${N_REQ}, run=${r}" >&2
#         continue
#       fi

#       time_us=$(echo "${line}" | grep -oE '[0-9]+')
#       echo "${n},0,${N_REQ},${r},${time_us}" >> "${OUT_CSV}"
#     done
#   done
# done

echo "== Измерение конвейерной версии (lab05_pipeline) с DEFAULT_THREADS из Config.h ==" >&2

# Пытаемся извлечь значение DEFAULT_THREADS из заголовка Config.h,
# чтобы записать его в CSV для наглядности.
DEFAULT_THREADS=$(grep -E "DEFAULT_THREADS" include/Config.h | grep -oE "[0-9]+" | head -n1 || echo 1)
echo "  -> DEFAULT_THREADS = ${DEFAULT_THREADS}" >&2

for n in "${SIZES[@]}"; do
  graph="../graphs/graph_${n}.dot"
  targets=$(cat "../graphs/graph_${n}_targets.txt")

  for N_REQ in "${N_VALUES[@]}"; do
    echo "  -> vertices=${n}, N=${N_REQ} (конвейер)" >&2
    for r in $(seq 1 "${REPEATS}"); do
      line=$("${BUILD_DIR}/lab05_pipeline" "${graph}" 0 "${targets}" "${N_REQ}" | \
             grep "Общее время конвейера" || true)

      if [[ -z "${line}" ]]; then
        echo "Не удалось извлечь время для конвейерной версии, n=${n}, N=${N_REQ}, run=${r}" >&2
        continue
      fi

      time_us=$(echo "${line}" | grep -oE '[0-9]+')
      echo "${n},${DEFAULT_THREADS},${N_REQ},${r},${time_us}" >> "${OUT_CSV}"
    done
  done
done

echo "Результаты эксперимента сохранены в ${OUT_CSV}" >&2

echo "== Вычисление среднего времени ==" >&2
python3 average_results.py
echo "== Конец эксперимента ==" >&2
