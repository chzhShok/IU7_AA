#pragma once

#include <cstdint>
#include <vector>

#include "Config.h"

class DijkstraResult {
public:
    std::vector<uint64_t> dist;
    std::vector<int> parent;
};

class Graph;

class DijkstraSequential {
public:
    DijkstraSequential(const Graph &g, int start);
    DijkstraResult run();

private:
    const Graph &g_;
    int start_;
};


