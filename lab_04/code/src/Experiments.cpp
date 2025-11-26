#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "DijkstraPar.h"
#include "DijkstraSeq.h"
#include "Experiments.h"
#include "Graph.h"
#include "Timer.h"

#include <map>

void ExperimentRunner::run_comparative_analysis() {
    unsigned int logical_cores = std::thread::hardware_concurrency();
    unsigned int physical_cores = logical_cores;

    std::cout << "=== ХАРАКТЕРИСТИКИ СИСТЕМЫ ===" << std::endl;
    std::cout << "Логические ядра: " << logical_cores << std::endl;
    std::cout << "Физические ядра: " << physical_cores << std::endl;

    std::vector<GraphInfo> test_graphs = generate_test_graphs();

    std::vector<ExperimentResult> results;

    std::cout << "\n=== СРАВНЕНИЕ С РАЗНЫМ КОЛИЧЕСТВОМ ПОТОКОВ ===" << std::endl;
    std::vector<int> thread_counts = generate_thread_counts(logical_cores);

    for (const auto &graph_info: test_graphs) {
        std::cout << "\nГраф " << graph_info.vertex_count << " вершин:" << std::endl;

        for (int threads: thread_counts) {
            auto result = run_experiment_series(graph_info, threads, 3);
            results.push_back(result);

            std::cout << "  Потоки=" << threads << ": " << result.time_us << " us" << std::endl;
        }
    }

    save_results_to_csv(results, "experiment_results.csv");

    analyze_and_recommend(results, logical_cores);
}

std::vector<ExperimentRunner::GraphInfo> ExperimentRunner::generate_test_graphs() {
    std::vector<int> sizes = {3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};
    std::vector<GraphInfo> graphs;

    for (int size: sizes) {
        std::string filename = "data/graph_" + std::to_string(size) + ".dot";
        std::string targets_filename = "data/graph_" + std::to_string(size) + "_targets.txt";

        if (!std::ifstream(filename)) {
            std::cout << "Генерация графа " << filename << std::endl;
            generate_graph(size, filename);
        }

        try {
            Graph g = Graph::load_from_dot(filename);
            int edge_count = count_edges(g);
            graphs.push_back({filename, size, edge_count});
        } catch (const std::exception &e) {
            std::cerr << "Ошибка загрузки графа " << filename << ": " << e.what() << std::endl;
        }
    }

    return graphs;
}

void ExperimentRunner::generate_graph(int vertices, const std::string &filename) {
    std::string command = "python3 generate_graph.py " + std::to_string(vertices) + " " + filename + " 0 -t > /dev/null";
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Ошибка генерации графа!" << std::endl;
    }
}

std::vector<int> ExperimentRunner::load_target_nodes(const Graph &g, const std::string &graph_filename) {
    std::string targets_filename = graph_filename;
    size_t dot_pos = targets_filename.find_last_of('.');
    if (dot_pos != std::string::npos) {
        targets_filename = targets_filename.substr(0, dot_pos) + "_targets.txt";
    } else {
        targets_filename += "_targets.txt";
    }

    std::ifstream file(targets_filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл целей: " << targets_filename << std::endl;
        return find_target_nodes(g, 10);
    }

    std::vector<int> targets;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string token;

        while (std::getline(ss, token, ',')) {
            token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());

            if (!token.empty()) {
                try {
                    int node_index = std::stoi(token);
                    if (node_index >= 0 && node_index < static_cast<int>(g.adj.size())) {
                        targets.push_back(node_index);
                    } else {
                        std::cerr << "Некорректный индекс вершины в файле целей: " << node_index << std::endl;
                    }
                } catch (const std::exception &e) {
                    auto node_id = g.find_node(token);
                    if (node_id) {
                        targets.push_back(*node_id);
                    } else {
                        std::cerr << "Не удалось найти вершину: " << token << std::endl;
                    }
                }
            }
        }
        break;
    }

    file.close();

    if (targets.empty()) {
        std::cerr << "Файл целей пуст или не содержит валидных целей, использую случайные цели" << std::endl;
        return find_target_nodes(g, 10);
    }

    return targets;
}

int ExperimentRunner::count_edges(const Graph &g) {
    int total_edges = 0;
    for (const auto &adj_list: g.adj) {
        total_edges += adj_list.size();
    }
    return total_edges;
}

std::vector<int> ExperimentRunner::generate_thread_counts(unsigned int logical_cores) {
    std::vector<int> counts = {0, 1, 2, 4};

    int max_threads = 8 * logical_cores;
    for (int threads = 8; threads <= max_threads; threads *= 2) {
        counts.push_back(threads);
    }

    if (std::find(counts.begin(), counts.end(), logical_cores) == counts.end()) {
        counts.push_back(logical_cores);
    }

    std::sort(counts.begin(), counts.end());
    return counts;
}

ExperimentRunner::ExperimentResult ExperimentRunner::run_experiment_series(const GraphInfo &graph_info, int threads, int runs) {
    std::vector<long long> times;

    for (int i = 0; i < runs; ++i) {
        try {
            Graph g = Graph::load_from_dot(graph_info.filename);
            int start_node = 0;
            std::vector<int> target_nodes = load_target_nodes(g, graph_info.filename);

            long long time = run_single_experiment(g, start_node, target_nodes, threads);
            times.push_back(time);

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } catch (const std::exception &e) {
            std::cerr << "Ошибка эксперимента: " << e.what() << std::endl;
        }
    }

    if (times.empty()) {
        std::cout << "times.empty" << std::endl;
        return {graph_info.vertex_count, threads, 0, threads == 0};
    }

    std::sort(times.begin(), times.end());
    long long avg_time = 0;
    for (int i = 0; i < (int) times.size(); ++i) {
        avg_time += times[i];
    }
    avg_time /= times.size();

    return {graph_info.vertex_count, threads, avg_time, threads == 0};
}

long long ExperimentRunner::run_single_experiment(const Graph &g, int start_node, const std::vector<int> &target_nodes, int threads) {
    long time = 0;

    if (threads == 0) {
        DijkstraSequential seq(g, start_node);
        Timer timer;
        auto result = seq.run();
        time = timer.us();
    } else {
        DijkstraParallel par(g, start_node, threads);
        Timer timer;
        auto result = par.run();
        time = timer.us();
    }

    return time;
}

int ExperimentRunner::find_start_node(const Graph &g) {
    int best_node = 0;
    size_t max_degree = 0;

    for (int i = 0; i < (int) g.adj.size(); ++i) {
        if (g.adj[i].size() > max_degree) {
            max_degree = g.adj[i].size();
            best_node = i;
        }
    }

    return best_node;
}

std::vector<int> ExperimentRunner::find_target_nodes(const Graph &g, int count) {
    std::vector<int> targets;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, g.adj.size() - 1);

    while (targets.size() < (size_t) count && targets.size() < g.adj.size()) {
        int node = dis(gen);
        if (std::find(targets.begin(), targets.end(), node) == targets.end()) {
            targets.push_back(node);
        }
    }

    return targets;
}

void ExperimentRunner::save_results_to_csv(const std::vector<ExperimentResult> &results, const std::string &filename) {
    std::ofstream file(filename);
    file << "graph_size,threads,time_us,is_sequential\n";

    for (const auto &result: results) {
        file << result.graph_size << ","
             << result.threads << ","
             << result.time_us << ","
             << (result.is_sequential ? "true" : "false") << "\n";
    }

    std::cout << "Результаты сохранены в " << filename << std::endl;
}

void ExperimentRunner::analyze_and_recommend(const std::vector<ExperimentResult> &results, unsigned int logical_cores) {
    std::cout << "\n=== АНАЛИЗ РЕЗУЛЬТАТОВ И РЕКОМЕНДАЦИИ ===" << std::endl;

    analyze_scalability(results, logical_cores);

    recommend_optimal_threads(results, logical_cores);
}

void ExperimentRunner::analyze_scalability(const std::vector<ExperimentResult> &results, unsigned int logical_cores) {
    std::cout << "\n--- Анализ масштабируемости ---" << std::endl;

    for (int size: {3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000}) {
        auto seq_it = std::find_if(results.begin(), results.end(),
                                   [size](const ExperimentResult &r) { return r.graph_size == size && r.is_sequential; });

        if (seq_it == results.end()) continue;

        std::cout << "Граф " << size << " вершин:" << std::endl;

        for (int threads: generate_thread_counts(logical_cores)) {
            if (threads == 0) continue;

            auto par_it = std::find_if(results.begin(), results.end(),
                                       [size, threads](const ExperimentResult &r) { return r.graph_size == size && r.threads == threads; });

            if (par_it != results.end()) {
                double speedup = (double) seq_it->time_us / par_it->time_us;
                double efficiency = speedup / threads * 100;

                std::cout << "  Потоки " << threads << ": ускорение=" << speedup
                          << "x, эффективность=" << efficiency << "%" << std::endl;
            }
        }
    }
}

int analyze_optimal_k(const std::vector<int> &best_threads, unsigned int logical_cores) {
    std::map<int, int> frequency;
    for (int threads: best_threads) {
        frequency[threads]++;
    }

    int most_frequent = 1;
    int max_count = 0;
    for (const auto &[threads, count]: frequency) {
        if (count > max_count) {
            max_count = count;
            most_frequent = threads;
        }
    }

    return most_frequent;
}

void ExperimentRunner::recommend_optimal_threads(const std::vector<ExperimentResult> &results, unsigned int logical_cores) {
    std::cout << "\n--- Рекомендация по выбору количества потоков ---" << std::endl;

    std::vector<int> test_sizes = {3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};

    std::map<int, int> best_threads_by_size;
    std::vector<int> all_best_threads;

    for (int size: test_sizes) {
        int best_threads = 1;
        long long best_time = std::numeric_limits<long long>::max();

        for (int threads: generate_thread_counts(logical_cores)) {
            if (threads == 0) continue;

            auto it = std::find_if(results.begin(), results.end(),
                                   [size, threads](const ExperimentResult &r) {
                                       return r.graph_size == size && r.threads == threads;
                                   });

            if (it != results.end() && it->time_us < best_time) {
                best_time = it->time_us;
                best_threads = threads;
            }
        }

        best_threads_by_size[size] = best_threads;
        all_best_threads.push_back(best_threads);

        std::cout << "Граф " << size << " вершин: оптимальное k = "
                  << best_threads << " (время: " << best_time << "us)" << std::endl;
    }

    int recommended_k = analyze_optimal_k(all_best_threads, logical_cores);

    std::cout << "\nОБЩАЯ РЕКОМЕНДАЦИЯ (на основе экспериментов):" << std::endl;
    std::cout << "Оптимальное количество потоков k = " << recommended_k << std::endl;
    std::cout << "Найдено в экспериментах: ";
    for (int threads: all_best_threads) {
        std::cout << threads << " ";
    }
    std::cout << std::endl;
}
