import matplotlib.pyplot as plt

# Данные
n_even = [100, 200, 300, 400, 500, 600, 700, 800]
classic_even = [x/1000 for x in [1713, 12684, 46280, 109879, 222837, 385627, 618165, 938280]]
winograd_even = [x/1000 for x in [1362, 10152, 37632, 90012, 179346, 314130, 503980, 766001]]
opt_winograd_even = [x/1000 for x in [1357, 10159, 37420, 89121, 168155, 292214, 481834, 732577]]

# Настройка стиля графиков
plt.style.use('default')
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 12

# Создание фигуры с графиком
fig, ax1 = plt.subplots(1, 1, figsize=(15, 6))

# График: Абсолютные значения времени выполнения
ax1.plot(n_even, classic_even, 'o-', linewidth=2, markersize=8, label='Классический', color='blue')
ax1.plot(n_even, winograd_even, 's-', linewidth=2, markersize=8, label='Виноград', color='red')
ax1.plot(n_even, opt_winograd_even, '^-', linewidth=2, markersize=8, label='Оптимизированный Виноград', color='green')

ax1.set_xlabel('Размер матрицы (n)')
ax1.set_ylabel('Время выполнения (мс)')
ax1.set_title('Время выполнения алгоритмов')
ax1.grid(True, alpha=0.3)
ax1.legend()
ax1.set_xlim(0, 850)

# Сохранение в SVG файл
plt.savefig('algorithms_comparison_even.png', format='png', dpi=300, bbox_inches='tight')

plt.show()