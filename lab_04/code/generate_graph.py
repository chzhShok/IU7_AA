import random
import sys
import os

def generate_large_graph(num_vertices=5000, max_edges_per_vertex=50, min_edges_per_vertex=None, max_weight=100):
    dot_content = ['digraph G {']

    for i in range(num_vertices):
        dot_content.append(f'  "{i}";')

    total_edges = 0
    available_vertices = list(range(num_vertices))
    for u in range(num_vertices):
        max_possible = min(max_edges_per_vertex, num_vertices - 1)
        if max_possible <= 0:
            continue
        baseline = min_edges_per_vertex if min_edges_per_vertex is not None else max(1, int(max_possible * 0.9))
        min_allowed = max(0, min(baseline, max_possible))
        num_edges = random.randint(min_allowed, max_possible)
        candidates = [v for v in available_vertices if v != u]
        for v in random.sample(candidates, num_edges):
            weight = random.randint(1, max_weight)
            dot_content.append(f'  "{u}" -> "{v}" [label={weight}];')
            total_edges += 1

    dot_content.append('}')

    return '\n'.join(dot_content), total_edges

def create_farthest_targets(num_vertices, start_vertex="0"):
    targets = []

    close_targets = [str(i) for i in range(1, min(11, num_vertices // 10))]

    mid_step = max(1, num_vertices // 20)
    mid_targets = [str(i) for i in range(num_vertices // 4, num_vertices, mid_step)][:10]

    far_targets = [str(i) for i in range(num_vertices - min(10, num_vertices // 10), num_vertices)]

    all_targets = list(set(close_targets + mid_targets + far_targets))
    if start_vertex in all_targets:
        all_targets.remove(start_vertex)

    return all_targets[:15]

def save_graph_to_file(filename, content):
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    with open(filename, 'w') as f:
        f.write(content)
    print(f"Граф сохранён в файл: {filename}")

def print_usage():
    print("Использование: python generate_graph.py <количество_вершин> [имя_файла] [стартовая_вершина]")
    print("Примеры:")
    print("  python generate_graph.py 1000")
    print("  python generate_graph.py 5000 large_graph.dot")
    print("  python generate_graph.py 10000 graph_10k.dot 0")
    print("\nОпции:")
    print("  -t, --targets   Создать файл с целевыми вершинами для поиска самой дальней")

def main():
    if len(sys.argv) < 2 or sys.argv[1] in ['-h', '--help']:
        print_usage()
        return

    create_targets = '-t' in sys.argv or '--targets' in sys.argv
    if create_targets:
        sys.argv = [arg for arg in sys.argv if arg not in ['-t', '--targets']]

    try:
        num_vertices = int(sys.argv[1])
        if num_vertices <= 0:
            print("Ошибка: количество вершин должно быть положительным числом")
            return
    except ValueError:
        print("Ошибка: количество вершин должно быть целым числом")
        return

    if len(sys.argv) >= 3 and not sys.argv[2].startswith('-'):
        filename = sys.argv[2]
    else:
        filename = f"data/graph_{num_vertices}.dot"

    start_vertex = "0"
    if len(sys.argv) >= 4 and not sys.argv[3].startswith('-'):
        start_vertex = sys.argv[3]

    if os.path.exists(filename):
        response = input(f"Файл '{filename}' уже существует. Перезаписать? (y/N): ")
        if response.lower() not in ['y', 'yes', 'д', 'да']:
            print("Отменено.")
            return

    max_edges_per_vertex = min(num_vertices - 1, 200)
    min_edges_per_vertex = None
    max_weight = 100

    print(f"Генерация графа с {num_vertices} вершинами...")
    graph_content, total_edges = generate_large_graph(
        num_vertices=num_vertices,
        max_edges_per_vertex=max_edges_per_vertex,
        min_edges_per_vertex=min_edges_per_vertex,
        max_weight=max_weight
    )

    save_graph_to_file(filename, graph_content)

    print(f"Всего вершин: {num_vertices}")
    print(f"Всего рёбер: {total_edges}")
    print(f"Средняя степень исхода: {total_edges / num_vertices:.2f}")
    print(f"Размер файла: {os.path.getsize(filename) / 1024 / 1024:.2f} MB")

    if create_targets:
        targets = create_farthest_targets(num_vertices, start_vertex)
        targets_file = filename.replace('.dot', '_targets.txt')
        filename = filename.replace('code/', '')

        with open(targets_file, 'w') as f:
            f.write(",".join(targets))

        print(f"\nСоздан файл с целевыми вершинами: {targets_file}")
        print(f"Цели: {', '.join(targets)}")
        print(f"\nДля запуска вашей программы используйте:")
        print(f'./build/lab04 {filename} "{start_vertex}" "{",".join(targets)}" <потоки>')

if __name__ == "__main__":
    main()
