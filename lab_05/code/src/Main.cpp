#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <filesystem>

#include "Config.h"
#include "DijkstraSeq.h"
#include "Graph.h"
#include "Timer.h"

static std::vector<std::string> split_csv(const std::string &s) {
    std::vector<std::string> out;
    std::string item;
    std::stringstream ss(s);

    while (std::getline(ss, item, ',')) {
        size_t start = item.find_first_not_of(" \t\n\r");
        size_t end = item.find_last_not_of(" \t\n\r");
        if (start != std::string::npos && end != std::string::npos) {
            std::string trimmed = item.substr(start, end - start + 1);
            if (!trimmed.empty()) {
                out.push_back(trimmed);
            }
        }
    }
    return out;
}

static std::vector<int> reconstruct_path(int target, const std::vector<int> &parent) {
    std::vector<int> path;
    for (int v = target; v != -1; v = parent[v]) {
        path.push_back(v);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

int main(int argc, char **argv) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <input_file> <start_vertex> \"<marked_vertices>\" <N>\n";
        return 1;
    }

    const std::string graph_file = argv[1];
    const std::string start_vertex_str = argv[2];
    const std::vector<std::string> target_vertex_strs = split_csv(argv[3]);

    if (target_vertex_strs.empty()) {
        std::cerr << "Error: at least one marked vertex must be specified\n";
        return 1;
    }

    int N = 0;
    try {
        N = std::stoi(argv[4]);
    } catch (const std::exception &e) {
        std::cerr << "Error: invalid N value: " << e.what() << "\n";
        return 1;
    }

    if (N <= 0) {
        std::cerr << "Error: N must be >= 1\n";
        return 1;
    }

    std::filesystem::path project_root = std::filesystem::current_path();
    if (project_root.filename() == "build") {
        project_root = project_root.parent_path().parent_path();
    } else if (project_root.filename() == "code") {
        project_root = project_root.parent_path();
    } else if (project_root.filename() == "lab_05") {
    } else {
        std::filesystem::path cur = project_root;
        while (!cur.empty()) {
            if (cur.filename() == "lab_05") {
                project_root = cur;
                break;
            }
            cur = cur.parent_path();
        }
    }

    std::filesystem::path result_dir = project_root / "result";
    std::error_code ec;
    std::filesystem::create_directories(result_dir, ec);

    Timer timer;

    for (int i = 0; i < N; ++i) {
        Graph g = Graph::load_from_dot(graph_file);

        auto start_id_opt = g.find_node(start_vertex_str);
        if (!start_id_opt) {
            throw std::runtime_error("Start node not found in graph: " + start_vertex_str);
        }
        int start_index = *start_id_opt;

        std::vector<int> target_indices;
        for (const auto &name: target_vertex_strs) {
            auto id = g.find_node(name);
            if (!id) {
                throw std::runtime_error("Target node not found in graph: " + name);
            }
            target_indices.push_back(*id);
        }

        DijkstraSequential seq(g, start_index);
        auto res = seq.run();

        std::filesystem::path in_path(graph_file);
        std::string base_name = in_path.filename().string();
        auto pos = base_name.find_last_of('.');
        if (pos == std::string::npos) {
            base_name += "_result_" + std::to_string(i) + ".txt";
        } else {
            base_name.insert(pos, "_result_" + std::to_string(i));
        }

        std::ofstream out(result_dir / base_name);

        out << "Стартовая вершина: " << start_vertex_str << "\n";
        out << "Помеченные вершины: ";
        for (size_t j = 0; j < target_vertex_strs.size(); ++j) {
            if (j) out << ", ";
            out << target_vertex_strs[j];
        }
        out << "\n\n";

        out << "Расстояния до помеченных вершин:\n";
        uint64_t best_dist = Config::INF_LIKE;
        int best_target_index = -1;
        std::vector<std::string> best_path_names;

        for (size_t j = 0; j < target_indices.size(); ++j) {
            int idx = target_indices[j];
            uint64_t d = res.dist[idx];
            out << "  " << g.idx_to_name[idx] << ": ";
            if (d >= Config::INF) {
                out << "INF\n";
            } else {
                out << d << "\n";
                if (d < best_dist) {
                    best_dist = d;
                    best_target_index = idx;
                    auto path = reconstruct_path(idx, res.parent);
                    best_path_names.clear();
                    for (int v: path) {
                        best_path_names.push_back(g.idx_to_name[v]);
                    }
                }
            }
        }

        out << "\nКратчайший путь среди помеченных вершин:\n";
        if (best_target_index == -1) {
            out << "  Все помеченные вершины недостижимы.\n";
        } else {
            out << "  Целевая вершина: " << g.idx_to_name[best_target_index] << "\n";
            out << "  Длина пути: " << best_dist << "\n";
            out << "  Путь: ";
            for (size_t j = 0; j < best_path_names.size(); ++j) {
                if (j) out << " -> ";
                out << best_path_names[j];
            }
            out << "\n";
        }
    }

    long long total_us = timer.us();
    std::cout << "Общее время последовательной обработки: " << total_us << " мкс\n";

    return 0;
}
