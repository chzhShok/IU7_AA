#include "ArgsParser.h"

#include <iostream>
#include <sstream>

ProgramArgs ArgsParser::parse(int argc, char **argv) {
    if (argc < 5) {
        print_usage(argv[0]);
        throw std::invalid_argument("Insufficient arguments");
    }

    ProgramArgs args;
    args.input_file = argv[1];
    args.start_node = argv[2];
    args.target_nodes = split_csv(argv[3]);

    try {
        args.threads = std::stoi(argv[4]);
    } catch (const std::exception &e) {
        throw std::invalid_argument("Invalid threads value: " + std::string(e.what()));
    }

    validate_args(args);
    return args;
}

void ArgsParser::validate_args(const ProgramArgs &args) {
    if (args.input_file.empty()) {
        throw std::invalid_argument("Input file path cannot be empty");
    }

    if (args.start_node.empty()) {
        throw std::invalid_argument("Start node cannot be empty");
    }

    if (args.target_nodes.empty()) {
        throw std::invalid_argument("At least one target node must be specified");
    }

    if (args.threads < 0) {
        throw std::invalid_argument("Thread count cannot be negative (use 0 for sequential)");
    }

    // Дополнительные проверки
    if (args.threads > 128) {
        throw std::invalid_argument("Thread count too high (max 128)");
    }

    // Проверка на дубликаты в целевых узлах
    for (size_t i = 0; i < args.target_nodes.size(); ++i) {
        for (size_t j = i + 1; j < args.target_nodes.size(); ++j) {
            if (args.target_nodes[i] == args.target_nodes[j]) {
                throw std::invalid_argument("Duplicate target node: " + args.target_nodes[i]);
            }
        }
    }
}

std::vector<std::string> ArgsParser::split_csv(const std::string &s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, ',')) {
        // Тримминг пробелов
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

void ArgsParser::print_usage(const std::string &program_name) {
    std::cerr << "Usage: " << program_name << " <input.dot> <start> <targets_csv> <threads>\n"
              << "\nArguments:\n"
              << "  input.dot    Path to graph file in DOT format\n"
              << "  start        Starting node name\n"
              << "  targets_csv  Comma-separated list of target nodes\n"
              << "  threads      Number of threads (0 for sequential, >0 for parallel)\n"
              << "\nExamples:\n"
              << "  " << program_name << " graph.dot A \"X,Y,Z\" 4\n"
              << "  " << program_name << " graph.dot \"Node A\" \"Target 1,Target 2\" 0\n";
}
