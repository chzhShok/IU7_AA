#include "DijkstraPar.h"
#include "DijkstraSeq.h"
#include "Graph.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <random>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

static int failures = 0;
static int passes = 0;
#define CHECK(cond)                                                                      \
    do {                                                                                 \
        if (!(cond)) {                                                                   \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n"; \
            ++failures;                                                                  \
        } else {                                                                         \
            ++passes;                                                                    \
        }                                                                                \
    } while (0)

static std::string write_temp(const std::string &content) {
    char templ[] = "/tmp/lab04_XXXXXX";
    int fd = mkstemp(templ);
    if (fd == -1) {
        perror("mkstemp");
        std::exit(1);
    }
    ssize_t n = write(fd, content.data(), content.size());
    (void) n;
    close(fd);
    return std::string(templ);
}

static std::vector<int> reconstruct_path(int target, const std::vector<int> &parent) {
    std::vector<int> path;
    for (int v = target; v != -1; v = parent[v]) path.push_back(v);
    std::reverse(path.begin(), path.end());
    return path;
}

static uint64_t sum_path_weight(const Graph &g, const std::vector<int> &path) {
    uint64_t sum = 0;
    for (size_t i = 1; i < path.size(); ++i) {
        int u = path[i - 1], v = path[i];
        bool found = false;
        for (auto [to, w]: g.adj[u])
            if (to == v) {
                sum += w;
                found = true;
                break;
            }
        if (!found) return UINT64_MAX;
    }
    return sum;
}

static void test_linear_graph() {
    std::string dot = R"(digraph G {
A -> B [label=1];
B -> C [weight=2];
C -> D [label=3];
}
)";
    auto path = write_temp(dot);
    Graph g = Graph::load_from_dot(path);

    int A = *g.find_node("A");
    int C = *g.find_node("C");
    int D = *g.find_node("D");

    DijkstraSequential seq(g, A);
    auto r = seq.run();

    CHECK(r.dist[A] == 0);
    CHECK(r.dist[C] == 3);
    CHECK(r.dist[D] == 6);

    auto pC = reconstruct_path(C, r.parent);
    CHECK(!pC.empty() && pC.front() == A && pC.back() == C);
    CHECK(sum_path_weight(g, pC) == r.dist[C]);
}

static void test_unreachable_and_defaults() {
    std::string dot = R"(digraph G {
A -> B;        // default 1
B -> C [weight=5];
C -> D [label=2, weight=100]; // use label=2
E; // isolated
}
)";
    auto path = write_temp(dot);
    Graph g = Graph::load_from_dot(path);

    int A = *g.find_node("A");
    DijkstraSequential seq(g, A);
    auto r = seq.run();

    CHECK(r.dist[A] == 0);
    // A->B (1), B->C (5), C->D (2)
    int D = *g.find_node("D");
    CHECK(r.dist[D] == 8);
}

static Graph make_random_graph(int n, int max_out, int max_w, uint32_t seed) {
    Graph g;
    for (int i = 0; i < n; ++i) {
        g.ensure_node(std::to_string(i));
    }
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> outd(0, max_out);
    std::uniform_int_distribution<int> vd(0, n - 1);
    std::uniform_int_distribution<int> wd(1, max_w);
    for (int u = 0; u < n; ++u) {
        int k = outd(rng);
        for (int i = 0; i < k; ++i) {
            int v = vd(rng);
            int w = wd(rng);
            g.add_edge(u, v, static_cast<uint32_t>(w));
        }
    }
    return g;
}

static void test_parallel_equals_sequential() {
    Graph g = make_random_graph(300, 5, 20, 42);
    int start = 0;
    DijkstraSequential seq(g, start);
    auto rs = seq.run();

    for (int th: {1, 2, 4}) {
        DijkstraParallel par(g, start, th);
        auto rp = par.run();
        CHECK(rp.dist.size() == rs.dist.size());
        for (size_t i = 0; i < rs.dist.size(); ++i) {
            if (rs.dist[i] != rp.dist[i]) {
                std::cerr << "Mismatch at i=" << i << " seq=" << rs.dist[i] << " par=" << rp.dist[i] << " threads=" << th << "\n";
            }
            CHECK(rs.dist[i] == rp.dist[i]);
        }
    }
}

// Новые тесты для функциональных тестов из отчета

static void test_small_graph_ABC() {
    std::string dot = R"(digraph G {
A -> B [label=2];
A -> C [weight=1];
}
)";
    auto path = write_temp(dot);
    Graph g = Graph::load_from_dot(path);

    int A = *g.find_node("A");
    int B = *g.find_node("B");
    int C = *g.find_node("C");

    // Последовательный алгоритм
    DijkstraSequential seq(g, A);
    auto rs = seq.run();

    CHECK(rs.dist[B] == 2);
    CHECK(rs.dist[C] == 1);

    // Параллельный алгоритм с разным числом потоков
    for (int threads : {1, 2, 4}) {
        DijkstraParallel par(g, A, threads);
        auto rp = par.run();
        CHECK(rp.dist[B] == 2);
        CHECK(rp.dist[C] == 1);
        CHECK(rs.dist[B] == rp.dist[B]);
        CHECK(rs.dist[C] == rp.dist[C]);
    }
}

static void test_bypass_path() {
    std::string dot = R"(digraph G {
A -> B [weight=3];
B -> C [label=1];
A -> C [weight=5];
}
)";
    auto path = write_temp(dot);
    Graph g = Graph::load_from_dot(path);

    int A = *g.find_node("A");
    int C = *g.find_node("C");

    DijkstraSequential seq(g, A);
    auto rs = seq.run();

    CHECK(rs.dist[C] == 4); // A->B->C = 3+1=4, а не A->C=5

    auto path_C = reconstruct_path(C, rs.parent);
    CHECK(path_C.size() == 3);
    CHECK(path_C[0] == A);
    CHECK(path_C[1] == *g.find_node("B"));
    CHECK(path_C[2] == C);

    // Проверка параллельной версии
    for (int threads : {1, 2, 4}) {
        DijkstraParallel par(g, A, threads);
        auto rp = par.run();
        CHECK(rp.dist[C] == 4);

        auto path_par = reconstruct_path(C, rp.parent);
        CHECK(path_par.size() == 3);
        CHECK(path_par[0] == A);
        CHECK(path_par[1] == *g.find_node("B"));
        CHECK(path_par[2] == C);
    }
}

static void test_linear_chain() {
    std::string dot = R"(digraph G {
A -> B [weight=1];
B -> C [label=1];
C -> D [weight=1];
}
)";
    auto path = write_temp(dot);
    Graph g = Graph::load_from_dot(path);

    int A = *g.find_node("A");
    int D = *g.find_node("D");

    DijkstraSequential seq(g, A);
    auto rs = seq.run();

    CHECK(rs.dist[D] == 3);

    auto path_D = reconstruct_path(D, rs.parent);
    CHECK(path_D.size() == 4);
    CHECK(path_D[0] == A);
    CHECK(path_D[1] == *g.find_node("B"));
    CHECK(path_D[2] == *g.find_node("C"));
    CHECK(path_D[3] == D);

    // Проверка параллельной версии
    for (int threads : {1, 2, 4, 8}) {
        DijkstraParallel par(g, A, threads);
        auto rp = par.run();
        CHECK(rp.dist[D] == 3);

        auto path_par = reconstruct_path(D, rp.parent);
        CHECK(path_par.size() == 4);
        CHECK(sum_path_weight(g, path_par) == 3);
    }
}

static void test_disconnected_graph() {
    std::string dot = R"(digraph G {
A -> B [weight=2];
C -> D [label=1];
}
)";
    auto path = write_temp(dot);
    Graph g = Graph::load_from_dot(path);

    int A = *g.find_node("A");
    int D = *g.find_node("D");

    DijkstraSequential seq(g, A);
    auto rs = seq.run();

    CHECK(rs.dist[D] >= Config::INF); // D недостижима из A

    // Проверка параллельной версии
    for (int threads : {1, 4}) {
        DijkstraParallel par(g, A, threads);
        auto rp = par.run();
        CHECK(rp.dist[D] >= Config::INF);
        CHECK(rs.dist[D] == rp.dist[D]);
    }
}

static void test_self_loop() {
    std::string dot = R"(digraph G {
A -> A [weight=1];
}
)";
    auto path = write_temp(dot);
    Graph g = Graph::load_from_dot(path);

    int A = *g.find_node("A");

    DijkstraSequential seq(g, A);
    auto rs = seq.run();

    CHECK(rs.dist[A] == 0); // Расстояние до себя = 0

    auto path_A = reconstruct_path(A, rs.parent);
    CHECK(path_A.size() == 1);
    CHECK(path_A[0] == A);

    // Проверка параллельной версии
    for (int threads : {1, 2}) {
        DijkstraParallel par(g, A, threads);
        auto rp = par.run();
        CHECK(rp.dist[A] == 0);

        auto path_par = reconstruct_path(A, rp.parent);
        CHECK(path_par.size() == 1);
        CHECK(path_par[0] == A);
    }
}

static void test_multiple_targets_shortest() {
    std::string dot = R"(digraph G {
A -> B [weight=2];
A -> C [weight=1];
A -> D [weight=3];
}
)";
    auto path = write_temp(dot);
    Graph g = Graph::load_from_dot(path);

    int A = *g.find_node("A");
    int B = *g.find_node("B");
    int C = *g.find_node("C");
    int D = *g.find_node("D");

    DijkstraSequential seq(g, A);
    auto rs = seq.run();

    // Проверка расстояний до всех целей
    CHECK(rs.dist[B] == 2);
    CHECK(rs.dist[C] == 1);
    CHECK(rs.dist[D] == 3);

    // C имеет минимальное расстояние
    uint64_t min_dist = std::min({rs.dist[B], rs.dist[C], rs.dist[D]});
    CHECK(min_dist == rs.dist[C]);

    // Проверка параллельной версии
    for (int threads : {1, 4}) {
        DijkstraParallel par(g, A, threads);
        auto rp = par.run();
        CHECK(rp.dist[B] == 2);
        CHECK(rp.dist[C] == 1);
        CHECK(rp.dist[D] == 3);
        CHECK(rs.dist[B] == rp.dist[B]);
        CHECK(rs.dist[C] == rp.dist[C]);
        CHECK(rs.dist[D] == rp.dist[D]);
    }
}

int main() {
    test_linear_graph();
    test_unreachable_and_defaults();
    test_parallel_equals_sequential();

    // Новые тесты соответствующие таблицам в отчете
    test_small_graph_ABC();           // Тест 1 из таблицы
    test_bypass_path();               // Тест 2 из таблицы
    test_linear_chain();              // Тест 3 из таблицы
    test_disconnected_graph();        // Тест 4 из таблицы
    test_self_loop();                 // Тест 5 из таблицы
    test_multiple_targets_shortest(); // Тест на несколько целей

    std::cout << "Tests passed: " << passes << ", failed: " << failures << "\n";
    return failures == 0 ? 0 : 1;
}
