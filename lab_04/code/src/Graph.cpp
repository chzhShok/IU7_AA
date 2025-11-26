#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

#include "Graph.h"

int Graph::ensure_node(const std::string &name) {
    auto it = name_to_idx.find(name);
    if (it != name_to_idx.end()) {
        return it->second;
    }

    int idx = static_cast<int>(adj.size());
    name_to_idx.emplace(name, idx);
    idx_to_name.push_back(name);
    adj.emplace_back();

    return idx;
}

std::optional<int> Graph::find_node(const std::string &name) const {
    auto it = name_to_idx.find(name);
    if (it == name_to_idx.end()) {
        return std::nullopt;
    }

    return it->second;
}

void Graph::add_edge(int u, int v, uint32_t w) {
    if (u < 0 || u >= static_cast<int>(adj.size()) || v < 0 || v >= static_cast<int>(adj.size())) {
        throw std::out_of_range("Node index out of bounds: " + std::to_string(u) + " or " + std::to_string(v));
    }

    adj[u].emplace_back(v, w);
}

static std::string trim(const std::string &s) {
    size_t i = 0, j = s.size();
    while (i < j && isspace(static_cast<unsigned char>(s[i]))) {
        ++i;
    }
    while (j > i && isspace(static_cast<unsigned char>(s[j - 1]))) {
        --j;
    }

    return s.substr(i, j - i);
}

static std::string unquote(const std::string &s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return s.substr(1, s.size() - 2);
    }

    return s;
}

Graph Graph::load_from_dot(const std::string &path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Failed to open DOT file: " + path);
    }

    std::stringstream buf;
    buf << in.rdbuf();
    std::string text = buf.str();

    Graph g;

    std::regex edge_re(R"(("[^"]+"|[A-Za-z0-9_]+)\s*->\s*("[^"]+"|[A-Za-z0-9_]+)\s*(\[(.*?)\])?\s*;)");
    std::regex node_re(R"(^\s*("[^"]+"|[A-Za-z0-9_]+)\s*;\s*$)");
    std::regex weight_kv_re(R"((label|weight)\s*=\s*([0-9]+))");

    if (text.find("digraph") == std::string::npos) {
        throw std::runtime_error("DOT must be a digraph with '->' arcs");
    }

    {
        std::stringstream ss(text);
        std::string line;
        while (std::getline(ss, line)) {
            std::smatch m;
            if (std::regex_search(line, m, node_re)) {
                std::string name = unquote(trim(m[1].str()));
                g.ensure_node(name);
            }
        }
    }

    {
        auto it = std::sregex_iterator(text.begin(), text.end(), edge_re);
        auto end = std::sregex_iterator();
        for (; it != end; ++it) {
            const std::smatch &m = *it;
            std::string u_name = unquote(m[1].str());
            std::string v_name = unquote(m[2].str());
            std::string attrs = m[4].matched ? m[4].str() : std::string();
            uint32_t w = 1;
            if (!attrs.empty()) {
                std::smatch wm;
                if (std::regex_search(attrs, wm, weight_kv_re)) {
                    unsigned long long val = std::stoull(wm[2].str());
                    if (val > UINT32_MAX) {
                        throw std::runtime_error("Weight exceeds 32-bit range");
                    }
                    w = static_cast<uint32_t>(val);
                }
            }

            int u = g.ensure_node(u_name);
            int v = g.ensure_node(v_name);
            g.add_edge(u, v, w);
        }
    }

    return g;
}
