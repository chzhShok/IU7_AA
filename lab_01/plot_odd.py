import matplotlib.pyplot as plt
import numpy as np

# Таблица 2: размеры 101-801 (нечетные)
n_odd = [101, 201, 301, 401, 501, 601, 701, 801]
classic_odd = [x/1000 for x in [1774, 13426, 48758, 114676, 229696, 396448, 630847, 952198]]
winograd_odd = [x/1000 for x in [1437, 10755, 39697, 94518, 186372, 322997, 513647, 775787]]
opt_winograd_odd = [x/1000 for x in [1423, 10730, 39373, 93645, 175773, 310996, 501286, 742479]]

# Настройка стиля графиков
plt.style.use('default')
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 12

# Создание фигуры с графиком
fig, ax1 = plt.subplots(1, 1, figsize=(15, 6))

# График: Абсолютные значения времени выполнения
ax1.plot(n_odd, classic_odd, 'o-', linewidth=2, markersize=8, label='Классический', color='blue')
ax1.plot(n_odd, winograd_odd, 's-', linewidth=2, markersize=8, label='Виноград', color='red')
ax1.plot(n_odd, opt_winograd_odd, '^-', linewidth=2, markersize=8, label='Оптимизированный Виноград', color='green')

ax1.set_xlabel('Размер матрицы (n)')
ax1.set_ylabel('Время выполнения (мс)')
ax1.set_title('Время выполнения алгоритмов')
ax1.grid(True, alpha=0.3)
ax1.legend()
ax1.set_xlim(0, 850)

# Сохранение в SVG файл
plt.savefig('algorithms_comparison_odd.png', format='png', dpi=300, bbox_inches='tight')

plt.show()