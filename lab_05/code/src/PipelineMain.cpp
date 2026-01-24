#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <filesystem>

#include "Config.h"
#include "DijkstraPar.h"
#include "Graph.h"


using Clock = std::chrono::high_resolution_clock;
using Microseconds = std::chrono::microseconds;

struct Request {
    int id{};
    std::string graph_file;
    std::string start_node_name;
    std::vector<std::string> target_node_names;

    // после ОУ1
    Graph graph;
    int start_index{-1};
    std::vector<int> target_indices;

    // после ОУ2
    std::vector<uint64_t> dist;
    std::vector<int> parent;
};

enum class EventType {
    Start,
    End
};

struct Event {
    long long time_us{};
    int request_id{};
    int device_id{};
    EventType type{};
};

template<typename T>
class BlockingQueue {
public:
    void push(T value) {
        {
            std::lock_guard<std::mutex> lg(m_);
            q_.push(std::move(value));
        }
        cv_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [&] { return !q_.empty(); });
        T v = std::move(q_.front());
        q_.pop();
        return v;
    }

private:
    std::queue<T> q_;
    std::mutex m_;
    std::condition_variable cv_;
};

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
        std::cerr << "Usage: " << argv[0] << " <input_file> <start_vertex> \"<marked_vertices>\" <N>\n";
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

    const int k_threads = Config::DEFAULT_THREADS;

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

    BlockingQueue<std::shared_ptr<Request>> q1, q2, q3;
    std::vector<Event> events;
    std::mutex events_mutex;

    const auto t0 = Clock::now();

    auto log_event = [&](int device_id, int request_id, EventType type) {
        const auto now = Clock::now();
        long long dt = std::chrono::duration_cast<Microseconds>(now - t0).count();
        Event ev;
        ev.time_us = dt;
        ev.request_id = request_id;
        ev.device_id = device_id;
        ev.type = type;
        std::lock_guard<std::mutex> lg(events_mutex);
        events.push_back(ev);
    };

    // генератор
    for (int i = 0; i < N; ++i) {
        auto req = std::make_shared<Request>();
        req->id = i;
        req->graph_file = graph_file;
        req->start_node_name = start_vertex_str;
        req->target_node_names = target_vertex_strs;
        q1.push(req);
    }

    // ОУ1
    std::thread ou1([&]() {
        for (int i = 0; i < N; ++i) {
            auto req = q1.pop();
            log_event(1, req->id, EventType::Start);

            req->graph = Graph::load_from_dot(req->graph_file);

            auto start_id_opt = req->graph.find_node(req->start_node_name);
            if (!start_id_opt) {
                throw std::runtime_error("Start node not found in graph: " + req->start_node_name);
            }
            req->start_index = *start_id_opt;

            req->target_indices.clear();
            for (const auto &name: req->target_node_names) {
                auto id = req->graph.find_node(name);
                if (!id) {
                    throw std::runtime_error("Target node not found in graph: " + name);
                }
                req->target_indices.push_back(*id);
            }

            log_event(1, req->id, EventType::End);
            q2.push(req);
        }
    });

    // ОУ2
    std::thread ou2([&]() {
        for (int i = 0; i < N; ++i) {
            auto req = q2.pop();
            log_event(2, req->id, EventType::Start);

            DijkstraParallel par(req->graph, req->start_index, k_threads);
            auto res = par.run();
            req->dist = std::move(res.dist);
            req->parent = std::move(res.parent);

            log_event(2, req->id, EventType::End);
            q3.push(req);
        }
    });

    // ОУ3
    std::thread ou3([&]() {
        for (int i = 0; i < N; ++i) {
            auto req = q3.pop();
            log_event(3, req->id, EventType::Start);

            std::filesystem::path in_path(req->graph_file);
            std::string base_name = in_path.filename().string();
            auto pos = base_name.find_last_of('.');
            if (pos == std::string::npos) {
                base_name += "_result_" + std::to_string(req->id) + ".txt";
            } else {
                base_name.insert(pos, "_result_" + std::to_string(req->id));
            }

            std::ofstream out(result_dir / base_name);

            out << "Стартовая вершина: " << req->start_node_name << "\n";
            out << "Помеченные вершины: ";
            for (size_t j = 0; j < req->target_node_names.size(); ++j) {
                if (j) out << ", ";
                out << req->target_node_names[j];
            }
            out << "\n\n";

            out << "Расстояния до помеченных вершин:\n";
            uint64_t best_dist = Config::INF_LIKE;
            int best_target_index = -1;
            std::vector<std::string> best_path_names;

            for (size_t j = 0; j < req->target_indices.size(); ++j) {
                int idx = req->target_indices[j];
                uint64_t d = req->dist[idx];
                out << "  " << req->graph.idx_to_name[idx] << ": ";
                if (d >= Config::INF) {
                    out << "INF\n";
                } else {
                    out << d << "\n";
                    if (d < best_dist) {
                        best_dist = d;
                        best_target_index = idx;
                        auto path = reconstruct_path(idx, req->parent);
                        best_path_names.clear();
                        for (int v: path) {
                            best_path_names.push_back(req->graph.idx_to_name[v]);
                        }
                    }
                }
            }

            out << "\nКратчайший путь среди помеченных вершин:\n";
            if (best_target_index == -1) {
                out << "  Все помеченные вершины недостижимы.\n";
            } else {
                out << "  Целевая вершина: " << req->graph.idx_to_name[best_target_index] << "\n";
                out << "  Длина пути: " << best_dist << "\n";
                out << "  Путь: ";
                for (size_t j = 0; j < best_path_names.size(); ++j) {
                    if (j) out << " -> ";
                    out << best_path_names[j];
                }
                out << "\n";
            }

            out.close();
            log_event(3, req->id, EventType::End);
        }
    });

    ou1.join();
    ou2.join();
    ou3.join();

    long long pipeline_time_us = std::chrono::duration_cast<Microseconds>(Clock::now() - t0).count();

    std::sort(events.begin(), events.end(),
    [](const Event &a, const Event &b) {
        return a.time_us < b.time_us;
    });


    auto event_type_str = [](EventType t) -> const char * {
        return t == EventType::Start ? "START" : "END";
    };

    auto device_str = [](int id) -> const char * {
        switch (id) {
            case 1: return "ОУ1";
            case 2: return "ОУ2";
            case 3: return "ОУ3";
            default: return "ОУ?";
        }
    };

    for (const auto &e: events) {
        std::cout << "[" << e.time_us << "] "
                  << event_type_str(e.type)
                  << " | Заявка#" << e.request_id
                  << " | " << device_str(e.device_id)
                  << "\n";
    }

    std::cout << "Общее время конвейера: " << pipeline_time_us << " мкс\n";

    return 0;
}
