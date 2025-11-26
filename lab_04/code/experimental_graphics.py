import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
from matplotlib.ticker import MultipleLocator

plt.style.use('seaborn-v0_8')
sns.set_palette("husl")

MARKERS = ['o', 's', '^', 'D', 'v', 'P', 'X', '*', 'h', '+']
COLOR_PALETTE = sns.color_palette("husl", 20)


def load_experiment_data(filename='experiment_results.csv'):
    df = pd.read_csv(filename)

    graph_sizes = sorted(df['graph_size'].unique())
    threads = sorted(df[df['threads'] > 0]['threads'].unique())

    data = {
        'vertices': graph_sizes,
        'threads': [0] + threads,
        'times': [],
        'speedup': [],
        'efficiency': []
    }

    for graph_size in graph_sizes:
        size_data = df[df['graph_size'] == graph_size]
        times = []
        for thread_count in data['threads']:
            time_val = size_data[size_data['threads'] == thread_count]['time_us'].values[0]
            times.append(time_val)
        data['times'].append(times)

    for i, graph_size in enumerate(graph_sizes):
        sequential_time = data['times'][i][0]
        speedup_row = []
        efficiency_row = []

        for j, thread_count in enumerate(threads):
            parallel_time = data['times'][i][j + 1]
            speedup = sequential_time / parallel_time
            efficiency = (speedup / thread_count) * 100

            speedup_row.append(speedup)
            efficiency_row.append(efficiency)

        data['speedup'].append(speedup_row)
        data['efficiency'].append(efficiency_row)

    return data, threads


data, threads_for_parallel = load_experiment_data('experiment_results.csv')


def plot_time_comparison():
    fig, ax = plt.subplots(figsize=(12, 8))

    for i, vertices in enumerate(data['vertices']):
        times_us = data['times'][i]
        times_ms = [t / 1000.0 for t in times_us[1:]]
        ax.plot(threads_for_parallel, times_ms,
                marker=MARKERS[i % len(MARKERS)],
                linewidth=2,
                markersize=6,
                label=f'{vertices} вершин',
                color=COLOR_PALETTE[i % len(COLOR_PALETTE)])

    ax.set_xlabel('Количество потоков', fontsize=12, fontweight='bold')
    ax.set_ylabel('Время выполнения (мс)', fontsize=12, fontweight='bold')
    ax.set_title('Сравнение времени выполнения при различном количестве потоков', fontsize=14, fontweight='bold', pad=20)

    ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    ax.grid(True, alpha=0.3)
    ax.set_xscale('log', base=2)
    ax.set_xticks(threads_for_parallel)
    ax.set_xticklabels([str(t) for t in threads_for_parallel])

    plt.tight_layout()
    plt.savefig('time_comparison.png', dpi=300, bbox_inches='tight')


def plot_speedup_analysis():
    fig, ax1 = plt.subplots(1, 1, figsize=(10, 6))

    for i, vertices in enumerate(data['vertices']):
        speedup = data['speedup'][i]
        ax1.plot(threads_for_parallel, speedup,
                 marker=MARKERS[i % len(MARKERS)],
                 linewidth=2,
                 markersize=6,
                 label=f'{vertices} вершин',
                 color=COLOR_PALETTE[i % len(COLOR_PALETTE)])

    ax1.set_xlabel('Количество потоков', fontsize=12, fontweight='bold')
    ax1.set_ylabel('Ускорение', fontsize=12, fontweight='bold')
    ax1.set_title('Анализ ускорения параллельного алгоритма', fontsize=14, fontweight='bold', pad=20)
    ax1.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    ax1.grid(True, alpha=0.3)
    ax1.set_xscale('log', base=2)
    ax1.set_xticks(threads_for_parallel)
    ax1.set_xticklabels([str(t) for t in threads_for_parallel])

    plt.tight_layout()
    plt.savefig('scalability_analysis.png', dpi=300, bbox_inches='tight')


def plot_sequential_vs_parallel():
    sequential_times_us = [data['times'][i][0] for i in range(len(data['vertices']))]

    thread_1_index = data['threads'].index(1)
    thread_4_index = data['threads'].index(4)

    parallel_1_thread_us = [data['times'][i][thread_1_index] for i in range(len(data['vertices']))]
    parallel_4_threads_us = [data['times'][i][thread_4_index] for i in range(len(data['vertices']))]

    sequential_times = [t / 1000.0 for t in sequential_times_us]
    parallel_1_thread = [t / 1000.0 for t in parallel_1_thread_us]
    parallel_4_threads = [t / 1000.0 for t in parallel_4_threads_us]

    fig, ax = plt.subplots(figsize=(12, 8))

    width = 0.25
    x = np.arange(len(data['vertices']))

    ax.bar(x - width, sequential_times, width, label='Последовательная (0 потоков)', alpha=0.8, color=COLOR_PALETTE[0])
    ax.bar(x, parallel_1_thread, width, label='Параллельная (1 поток)', alpha=0.8, color=COLOR_PALETTE[1])
    ax.bar(x + width, parallel_4_threads, width, label='Параллельная (4 потока)', alpha=0.8, color=COLOR_PALETTE[2])

    ax.set_xlabel('Количество вершин в графе', fontsize=12, fontweight='bold')
    ax.set_ylabel('Время выполнения (мс)', fontsize=12, fontweight='bold')
    ax.set_title('Сравнение последовательного и параллельного алгоритмов', fontsize=14, fontweight='bold', pad=20)
    ax.set_xticks(x)
    ax.set_xticklabels([str(v) for v in data['vertices']])
    ax.legend()
    ax.grid(True, alpha=0.3, axis='y')

    for i, v in enumerate(data['vertices']):
        speedup_1 = sequential_times[i] / parallel_1_thread[i]
        speedup_4 = sequential_times[i] / parallel_4_threads[i]
        ax.text(i, parallel_1_thread[i] * 1.05, f'{speedup_1:.1f}x', ha='center', fontweight='bold')
        ax.text(i + width, parallel_4_threads[i] * 1.05, f'{speedup_4:.1f}x', ha='center', fontweight='bold')

    plt.tight_layout()
    plt.savefig('sequential_vs_parallel.png', dpi=300, bbox_inches='tight')


def print_statistics():
    print("=== СТАТИСТИКА ЭКСПЕРИМЕНТОВ ===")
    print(f"Размеры графов: {data['vertices']}")
    print(f"Количество потоков: {threads_for_parallel}")
    print(f"Количество измерений: {len(data['vertices']) * len(data['threads'])}")

    print("\n=== МАКСИМАЛЬНОЕ УСКОРЕНИЕ ПО РАЗМЕРАМ ===")
    for i, vertices in enumerate(data['vertices']):
        max_speedup = max(data['speedup'][i])
        optimal_thread = threads_for_parallel[data['speedup'][i].index(max_speedup)]
        print(f"Граф {vertices} вершин: ускорение {max_speedup:.2f}x при {optimal_thread} потоках")


if __name__ == "__main__":
    print("Загрузка данных из experiment_results.csv...")

    print_statistics()

    print("\nСоздание графиков для экспериментальной части...")

    plot_time_comparison()
    print("График сравнения времени выполнения создан")

    plot_speedup_analysis()
    print("График анализа ускорения и эффективности создан")

    plot_sequential_vs_parallel()
    print("График сравнения последовательной и параллельной версий создан")
