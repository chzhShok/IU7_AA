#pragma once

#include <vector>

#include "Config.h"

class Graph;

class DijkstraParResult {
public:
    std::vector<uint64_t> dist;
    std::vector<int> parent;
};

class DijkstraParallel {
public:
    DijkstraParallel(const Graph& g, int start, int threads);
    void set_threads(int t);
    DijkstraParResult run();
private:
    const Graph& g_;
    int start_;
    int threads_;
};
