#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "Config.h"
#include "Graph.h"

class JsonResultBuilder {
public:
    JsonResultBuilder() = default;

    void build(const Graph &g,
               const std::string &start_name,
               const std::vector<std::string> &target_names,
               const std::vector<int> &target_ids,
               const std::vector<uint64_t> &dist,
               const std::vector<int> &parent,
               int threads,
               long long elapsed,
               bool use_seq);

    std::string get_result() const {
        return out_.str();
    }

    void clear() {
        out_.str("");
        out_.clear();
    }

private:
    std::ostringstream out_;

    void escape_json(const std::string &s);
    void build_header(const std::string &start_name,
                      const std::vector<std::string> &target_names,
                      int threads, long long elapsed, bool use_seq);
    void build_distances(const std::vector<std::string> &target_names,
                         const std::vector<int> &target_ids,
                         const std::vector<uint64_t> &dist);
    void build_shortest_path(const Graph &g,
                             const std::vector<std::string> &target_names,
                             const std::vector<int> &target_ids,
                             const std::vector<uint64_t> &dist,
                             const std::vector<int> &parent);
    std::vector<int> reconstruct_path(int target, const std::vector<int> &parent) const;
};
