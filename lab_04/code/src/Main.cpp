// Main.cpp (модифицированная версия)
#include "ArgsParser.h"
#include "Config.h"
#include "DijkstraPar.h"
#include "DijkstraSeq.h"
#include "Experiments.h"// Добавляем заголовок экспериментов
#include "Graph.h"
#include "JsonResultBuilder.h"
#include "Timer.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

static std::vector<int> map_targets(const Graph &g, const std::vector<std::string> &names) {
    std::vector<int> ids;
    ids.reserve(names.size());
    for (const auto &n: names) {
        auto id = g.find_node(n);
        if (!id) {
            throw std::runtime_error("Target node not found: " + n);
        }
        ids.push_back(*id);
    }
    return ids;
}

static std::string json_escape(const std::string &s) {
    std::string o;
    o.reserve(s.size() + 8);
    for (unsigned char c: s) {
        switch (c) {
            case '\\':
                o += "\\\\";
                break;
            case '"':
                o += "\\\"";
                break;
            case '\n':
                o += "\\n";
                break;
            case '\r':
                o += "\\r";
                break;
            case '\t':
                o += "\\t";
                break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    o += buf;
                } else
                    o += c;
        }
    }
    return o;
}

static void print_error_json(const std::string &msg) {
    std::cout << "{\"error\":\"" << json_escape(msg) << "\"}" << std::endl;
}

static void print_usage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "  Основной режим: lab04 <input.dot> <start> <targets_csv> <threads>" << std::endl;
    std::cout << "  Эксперименты:   lab04 -e" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  lab04 graph.dot A \"X,Y,Z\" 4" << std::endl;
    std::cout << "  lab04 graph.dot \"Node A\" \"Target 1,Target 2\" 0" << std::endl;
    std::cout << "  lab04 -e  # запуск сравнительных экспериментов" << std::endl;
}

int main(int argc, char **argv) {
    if (argc == 2 && std::string(argv[1]) == "-e") {
        std::cout << "Запуск сравнительных экспериментов..." << std::endl;
        ExperimentRunner runner;
        runner.run_comparative_analysis();
        return 0;
    }

    try {
        if (argc < 2) {
            print_usage();
            return 1;
        }

        ProgramArgs args = ArgsParser::parse(argc, argv);

        Graph g = Graph::load_from_dot(args.input_file);

        auto start_id_opt = g.find_node(args.start_node);
        if (!start_id_opt) {
            print_error_json("start node not found: " + args.start_node);
            return 1;
        }
        int start = *start_id_opt;
        auto target_ids = map_targets(g, args.target_nodes);

        bool use_seq = (args.threads == 0);
        std::vector<uint64_t> dist;
        std::vector<int> parent;
        long long elapsed;

        if (use_seq) {
            DijkstraSequential seq(g, start);
            Timer t;
            auto r = seq.run();
            elapsed = t.us();
            dist = std::move(r.dist);
            parent = std::move(r.parent);
        } else {
            DijkstraParallel par(g, start, args.threads);
            Timer t;
            auto r = par.run();
            elapsed = t.us();
            dist = std::move(r.dist);
            parent = std::move(r.parent);
        }

        JsonResultBuilder builder;
        builder.build(g, args.start_node, args.target_nodes, target_ids, dist, parent, args.threads, elapsed, use_seq);

        std::cout << builder.get_result() << std::endl;
        return 0;
    } catch (const std::exception &e) {
        print_error_json(e.what());
        return 1;
    }
}
