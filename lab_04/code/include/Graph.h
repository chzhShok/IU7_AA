#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Graph {
public:
    std::vector<std::vector<std::pair<int, uint32_t>>> adj;
    std::unordered_map<std::string, int> name_to_idx;
    std::vector<std::string> idx_to_name;

    int ensure_node(const std::string &name);
    std::optional<int> find_node(const std::string &name) const;

    void add_edge(int u, int v, uint32_t w);

    size_t size() const { return adj.size(); }

    static Graph load_from_dot(const std::string &path);
};
