#include "DijkstraSeq.h"
#include "Graph.h"

DijkstraSequential::DijkstraSequential(const Graph &g, int start)
    : g_(g), start_(start) {}

DijkstraResult DijkstraSequential::run() {
    const uint64_t INF = Config::INF;
    const int n = static_cast<int>(g_.adj.size());

    std::vector<uint64_t> dist(n, INF);
    std::vector<int> parent(n, -1);

    dist[start_] = 0;
    std::vector<char> used(n, 0);

    for (int iter = 0; iter < n; ++iter) {
        int u = -1;
        uint64_t best = INF;
        for (int i = 0; i < n; ++i) {
            if (!used[i] && dist[i] < best) {
                best = dist[i];
                u = i;
            }
        }
        if (u == -1 || best == INF) {
            break;
        }
        used[u] = 1;
        for (auto [v, w]: g_.adj[u]) {
            uint64_t nd = best + w;
            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;
            }
        }
    }

    return {std::move(dist), std::move(parent)};
}
