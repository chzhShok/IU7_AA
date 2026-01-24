#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <random>
#include <thread>
#include <vector>

#include "DijkstraPar.h"
#include "Graph.h"

namespace {
    class Node {
    public:
        uint64_t dist;
        int v;
        bool operator>(const Node &o) const {
            return dist > o.dist;
        }
    };

    class WorkQueue {
    public:
        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        std::mutex m;
        std::atomic<int> approx_size{0};
    };

}// namespace

DijkstraParallel::DijkstraParallel(const Graph &g, int start, int threads)
    : g_(g), start_(start), threads_(threads) {}

void DijkstraParallel::set_threads(int t) {
    threads_ = t;
}

DijkstraParResult DijkstraParallel::run() {
    int threads = threads_;
    if (threads <= 0) {
        threads = std::max(1u, std::thread::hardware_concurrency());
    }

    const int n = static_cast<int>(g_.adj.size());
    const uint64_t INF = Config::INF;

    std::vector<std::atomic<uint64_t>> dist(n);
    std::vector<std::atomic<int>> parent(n);
    for (int i = 0; i < n; ++i) {
        dist[i].store(INF, std::memory_order_relaxed);
        parent[i].store(-1, std::memory_order_relaxed);
    }

    std::vector<WorkQueue> queues(threads);
    std::atomic<long long> tasks{0};
    std::atomic<int> active{0};
    std::condition_variable cv;
    std::mutex cv_m;
    std::atomic<bool> done{false};

    auto random_thread = [threads]() {
        thread_local std::mt19937_64 gen{std::random_device{}() ^ ((uint64_t) std::hash<std::thread::id>{}(std::this_thread::get_id()))};
        std::uniform_int_distribution<int> dist(0, threads - 1);
        return dist(gen);
    };

    auto push_to = [&](int owner, const Node &nd) {
        if (owner < 0 || owner >= threads) {
            owner = 0;
        }

        {
            std::lock_guard<std::mutex> lg(queues[owner].m);
            queues[owner].pq.push(nd);
            queues[owner].approx_size.fetch_add(1, std::memory_order_relaxed);
        }
        tasks.fetch_add(1, std::memory_order_relaxed);
        cv.notify_one();
    };

    auto pop_local = [&](int idx, Node &out) -> bool {
        if (queues[idx].approx_size.load(std::memory_order_relaxed) == 0) {
            return false;
        }

        std::unique_lock<std::mutex> lk(queues[idx].m);
        if (queues[idx].pq.empty()) {
            queues[idx].approx_size.store(0, std::memory_order_relaxed);
            return false;
        }

        out = queues[idx].pq.top();
        queues[idx].pq.pop();
        queues[idx].approx_size.fetch_sub(1, std::memory_order_relaxed);
        return true;
    };

    auto steal_from = [&](int idx, Node &out) -> bool {
        if (threads <= 1) {
            return false;
        }

        int start = random_thread();
        for (int attempt = 0; attempt < threads; ++attempt) {
            int target = (start + attempt) % threads;
            if (target == idx) {
                continue;
            }

            if (queues[target].approx_size.load(std::memory_order_relaxed) == 0) {
                continue;
            }

            std::unique_lock<std::mutex> lk(queues[target].m, std::try_to_lock);
            if (!lk.owns_lock()) {
                continue;
            }

            if (queues[target].pq.empty()) {
                queues[target].approx_size.store(0, std::memory_order_relaxed);
                continue;
            }

            out = queues[target].pq.top();
            queues[target].pq.pop();
            queues[target].approx_size.fetch_sub(1, std::memory_order_relaxed);
            return true;
        }
        return false;
    };

    auto try_pop = [&](int idx, Node &out) -> bool {
        if (pop_local(idx, out)) {
            return true;
        }

        return steal_from(idx, out);
    };

    auto worker = [&](int idx) {
        Node cur;
        while (true) {
            if (!try_pop(idx, cur)) {
                if (tasks.load(std::memory_order_relaxed) == 0 && active.load(std::memory_order_relaxed) == 0) {
                    done.store(true, std::memory_order_relaxed);
                    cv.notify_all();
                    break;
                }

                std::unique_lock<std::mutex> lk(cv_m);
                cv.wait(lk, [&]() {
                    return tasks.load(std::memory_order_relaxed) > 0 || done.load(std::memory_order_relaxed);
                });

                if (tasks.load(std::memory_order_relaxed) == 0 && active.load(std::memory_order_relaxed) == 0) {
                    break;
                }

                continue;
            }

            tasks.fetch_sub(1, std::memory_order_relaxed);

            uint64_t curd = dist[cur.v].load(std::memory_order_relaxed);
            if (cur.dist != curd) {
                if (tasks.load(std::memory_order_relaxed) == 0 && active.load(std::memory_order_relaxed) == 0) {
                    done.store(true, std::memory_order_relaxed);
                    cv.notify_all();
                }
                continue;
            }

            active.fetch_add(1, std::memory_order_relaxed);

            for (auto [to, w]: g_.adj[cur.v]) {
                uint64_t nd = curd + w;
                uint64_t old = dist[to].load(std::memory_order_relaxed);

                while (nd < old) {
                    if (dist[to].compare_exchange_weak(old, nd, std::memory_order_relaxed)) {
                        parent[to].store(cur.v, std::memory_order_relaxed);
                        int owner = threads > 0 ? (to % threads) : 0;
                        push_to(owner, Node{nd, to});
                        break;
                    }
                }
            }

            active.fetch_sub(1, std::memory_order_relaxed);

            if (tasks.load(std::memory_order_relaxed) == 0 && active.load(std::memory_order_relaxed) == 0) {
                done.store(true, std::memory_order_relaxed);
                cv.notify_all();
            }
        }
    };

    dist[start_].store(0, std::memory_order_relaxed);
    int start_owner = threads > 0 ? (start_ % threads) : 0;
    push_to(start_owner, Node{0, start_});

    std::vector<std::thread> pool;
    pool.reserve(threads);
    for (int t = 0; t < threads; ++t) {
        pool.emplace_back(worker, t);
    }

    for (auto &th: pool) {
        th.join();
    }

    DijkstraParResult res;
    res.dist.resize(n);
    res.parent.resize(n);
    for (int i = 0; i < n; ++i) {
        res.dist[i] = dist[i].load(std::memory_order_relaxed);
        res.parent[i] = parent[i].load(std::memory_order_relaxed);
    }

    return res;
}
