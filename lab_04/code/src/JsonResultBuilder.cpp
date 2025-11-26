#include "JsonResultBuilder.h"
#include <algorithm>
#include <cstdio>

void JsonResultBuilder::build(const Graph &g,
                              const std::string &start_name,
                              const std::vector<std::string> &target_names,
                              const std::vector<int> &target_ids,
                              const std::vector<uint64_t> &dist,
                              const std::vector<int> &parent,
                              int threads,
                              long long elapsed,
                              bool use_seq) {
    out_.str("");
    out_.clear();

    out_ << "{";
    build_header(start_name, target_names, threads, elapsed, use_seq);
    out_ << ",";
    build_distances(target_names, target_ids, dist);
    out_ << ",";
    build_shortest_path(g, target_names, target_ids, dist, parent);
    out_ << "}";
}

void JsonResultBuilder::escape_json(const std::string &s) {
    for (unsigned char c: s) {
        switch (c) {
            case '\\':
                out_ << "\\\\";
                break;
            case '"':
                out_ << "\\\"";
                break;
            case '\n':
                out_ << "\\n";
                break;
            case '\r':
                out_ << "\\r";
                break;
            case '\t':
                out_ << "\\t";
                break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out_ << buf;
                } else {
                    out_ << c;
                }
        }
    }
}

void JsonResultBuilder::build_header(const std::string &start_name,
                                     const std::vector<std::string> &target_names,
                                     int threads, long long elapsed, bool use_seq) {
    out_ << "\"start\":\"";
    escape_json(start_name);
    out_ << "\",";

    out_ << "\"targets\":[";
    for (size_t i = 0; i < target_names.size(); ++i) {
        if (i) out_ << ",";
        out_ << "\"";
        escape_json(target_names[i]);
        out_ << "\"";
    }
    out_ << "],";

    out_ << "\"threads\":" << threads << ",";
    out_ << "\"algo\":\"" << (use_seq ? "seq" : "par") << "\",";
    out_ << "\"time_ms\":" << elapsed;
}

void JsonResultBuilder::build_distances(const std::vector<std::string> &target_names,
                                        const std::vector<int> &target_ids,
                                        const std::vector<uint64_t> &dist) {
    out_ << "\"distances\":{";
    for (size_t i = 0; i < target_names.size(); ++i) {
        if (i) out_ << ",";
        out_ << "\"";
        escape_json(target_names[i]);
        out_ << "\":";

        int v = target_ids[i];
        if (dist[v] >= Config::INF_LIKE) {
            out_ << "null";
        } else {
            out_ << dist[v];
        }
    }
    out_ << "}";
}

void JsonResultBuilder::build_shortest_path(const Graph &g,
                                            const std::vector<std::string> &target_names,
                                            const std::vector<int> &target_ids,
                                            const std::vector<uint64_t> &dist,
                                            const std::vector<int> &parent) {
    out_ << "\"shortest\":";

    uint64_t best = std::numeric_limits<uint64_t>::max();
    int best_tid = -1;
    for (size_t i = 0; i < target_ids.size(); ++i) {
        int v = target_ids[i];
        if (dist[v] < best) {
            best = dist[v];
            best_tid = static_cast<int>(i);
        }
    }

    if (best_tid == -1 || dist[target_ids[best_tid]] >= Config::INF_LIKE) {
        out_ << "null";
    } else {
        int tv = target_ids[best_tid];
        auto path_idx = reconstruct_path(tv, parent);

        out_ << "{\"target\":\"";
        escape_json(target_names[best_tid]);
        out_ << "\",";
        out_ << "\"distance\":" << dist[tv] << ",";
        out_ << "\"path\":[";

        for (size_t i = 0; i < path_idx.size(); ++i) {
            if (i) out_ << ",";
            out_ << "\"";
            escape_json(g.idx_to_name[path_idx[i]]);
            out_ << "\"";
        }
        out_ << "]}";
    }
}

std::vector<int> JsonResultBuilder::reconstruct_path(int target, const std::vector<int> &parent) const {
    std::vector<int> path;
    for (int v = target; v != -1; v = parent[v]) {
        path.push_back(v);
    }

    std::reverse(path.begin(), path.end());
    return path;
}
