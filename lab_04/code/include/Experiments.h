#pragma once

#include <vector>
#include <string>

class Graph;

class ExperimentRunner {
public:
    void run_comparative_analysis();

private:
    struct ExperimentResult {
        int graph_size;
        int threads;
        long long time_us;
        bool is_sequential;
    };

    struct GraphInfo {
        std::string filename;
        int vertex_count;
        int edge_count;
    };

    std::vector<GraphInfo> generate_test_graphs();
    void generate_graph(int vertices, const std::string& filename);
    int count_edges(const Graph& g);
    std::vector<int> generate_thread_counts(unsigned int logical_cores);
    ExperimentResult run_experiment_series(const GraphInfo& graph_info, int threads, int runs);
    long long run_single_experiment(const Graph& g, int start_node, const std::vector<int>& target_nodes, int threads);
    std::vector<int> load_target_nodes(const Graph& g, const std::string& graph_filename);
    int find_start_node(const Graph& g);
    std::vector<int> find_target_nodes(const Graph& g, int count);
    void save_results_to_csv(const std::vector<ExperimentResult>& results, const std::string& filename);
    void analyze_and_recommend(const std::vector<ExperimentResult>& results, unsigned int logical_cores);
    void analyze_overhead(const std::vector<ExperimentResult>& results);
    void analyze_scalability(const std::vector<ExperimentResult>& results, unsigned int logical_cores);
    void recommend_optimal_threads(const std::vector<ExperimentResult>& results, unsigned int logical_cores);
};
