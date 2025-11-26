import matplotlib.pyplot as plt

# Данные из таблицы
sizes = [100, 200, 300, 400, 500, 600, 700, 800, 900, 1000,
         1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000]

recursive_times = [2.649, 5.395, 6.700, 7.555, 8.273, 8.626, 9.629, 10.247,
                  10.489, 11.585, 12.170, 12.382, 14.295, 14.875, 16.320,
                  17.899, 18.396, 20.388, 20.858, 22.215]

iterative_times = [0.889, 1.453, 1.695, 2.109, 2.068, 2.362, 2.488, 2.783,
                  2.816, 2.957, 3.235, 3.464, 3.764, 4.018, 4.551, 4.787,
                  5.036, 5.435, 5.766, 6.014]

# Создание графика
plt.figure(figsize=(12, 8))
plt.plot(sizes, recursive_times, 'o-', linewidth=2, markersize=6, label='Рекурсивный алгоритм', color='red')
plt.plot(sizes, iterative_times, 's-', linewidth=2, markersize=6, label='Итеративный алгоритм', color='blue')

# Настройка внешнего вида
plt.xlabel('Размер последовательности', fontsize=12)
plt.ylabel('Время выполнения (микросекунды)', fontsize=12)
plt.title('Сравнение времени выполнения рекурсивного и итеративного алгоритмов', fontsize=14)
plt.legend(fontsize=12)
plt.grid(True, alpha=0.3)

# Улучшаем разметку осей
plt.xticks(range(0, 2100, 100), rotation=45)
plt.yticks(range(0, 25, 2))

# Автоматическое размещение подписей
plt.tight_layout()

# Показать график
# plt.show()

# Дополнительно: сохранение графика в файл
plt.savefig('algorithm_comparison.png', dpi=300, bbox_inches='tight')
print("График сохранен как 'algorithm_comparison.png'")
