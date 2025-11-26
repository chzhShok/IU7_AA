// ArgsParser.h (добавляем метод для экспериментов)
#pragma once

#include <stdexcept>
#include <string>
#include <vector>

struct ProgramArgs {
    std::string input_file;
    std::string start_node;
    std::vector<std::string> target_nodes;
    int threads;
    bool run_experiments = false;// Новый флаг

    bool valid() const {
        if (run_experiments) return true;
        return !input_file.empty() && !start_node.empty() && !target_nodes.empty() && threads >= 0;
    }
};

class ArgsParser {
public:
    static ProgramArgs parse(int argc, char **argv);

private:
    static void validate_args(const ProgramArgs &args);
    static std::vector<std::string> split_csv(const std::string &s);
    static void print_usage(const std::string &program_name);
};
