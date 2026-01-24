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
    print("Использование:")
    print("  python generate_graph.py <количество_вершин> [имя_файла] [стартовая_вершина]")
    print("  python generate_graph.py <n1,n2,n3,...> [стартовая_вершина]")
    print("\nПримеры:")
    print("  python generate_graph.py 1000")
    print("  python generate_graph.py 5000 ../graphs/graph_5000.dot")
    print("  python generate_graph.py 10000 ../graphs/graph_10000.dot 0")
    print("  python generate_graph.py 3000,4000,5000 0")
    print("\nОпции:")
    print("  -t, --targets   Создать файл с целевыми вершинами для поиска самой дальней")

def main():
    if len(sys.argv) < 2 or sys.argv[1] in ['-h', '--help']:
        print_usage()
        return

    create_targets = '-t' in sys.argv or '--targets' in sys.argv
    if create_targets:
        sys.argv = [arg for arg in sys.argv if arg not in ['-t', '--targets']]

    # Разбор списка чисел вершин:
    #  - "1000"         -> [1000]
    #  - "3000,4000,5"  -> [3000, 4000, 5]
    raw_sizes = sys.argv[1]
    vertices_list = []
    for part in raw_sizes.split(","):
        part = part.strip()
        if not part:
            continue
        try:
            v = int(part)
        except ValueError:
            print(f"Ошибка: '{part}' не является корректным целым числом вершин")
            return
        if v <= 0:
            print("Ошибка: количество вершин должно быть положительным числом")
            return
        vertices_list.append(v)

    if not vertices_list:
        print("Ошибка: не удалось распознать ни одного значения количества вершин")
        return

    # Если задан один размер, можно использовать пользовательское имя файла.
    # Если размеров несколько, имена файлов формируются автоматически: data/graph_<n>.dot
    single_graph = len(vertices_list) == 1

    start_vertex = "0"
    extra_arg_index = 2
    filename_override = None

    if single_graph and len(sys.argv) >= 3 and not sys.argv[2].startswith('-'):
        filename_override = sys.argv[2]
        extra_arg_index = 3

    if len(sys.argv) > extra_arg_index and not sys.argv[extra_arg_index].startswith('-'):
        start_vertex = sys.argv[extra_arg_index]

    for num_vertices in vertices_list:
        if single_graph and filename_override is not None:
            filename = filename_override
        else:
            filename = f"../graphs/graph_{num_vertices}.dot"

        if os.path.exists(filename):
            response = input(f"Файл '{filename}' уже существует. Перезаписать? (y/N): ")
            if response.lower() not in ['y', 'yes', 'д', 'да']:
                print("Отменено.")
                continue

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
            filename_hint = filename.replace('code/', '')

            with open(targets_file, 'w') as f:
                f.write(",".join(targets))

            print(f"\nСоздан файл с целевыми вершинами: {targets_file}")
            print(f"Цели: {', '.join(targets)}")
            print(f"\nДля запуска вашей программы используйте:")
            print(f'./build/lab04 {filename_hint} "{start_vertex}" "{",".join(targets)}" <потоки>')

if __name__ == "__main__":
    main()
