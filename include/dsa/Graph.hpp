#pragma once

#include <string>
#include <limits>
#include "dsa/Vector.hpp"
#include "dsa/HashTable.hpp"
#include "dsa/BinaryHeap.hpp"
#include "dsa/CircularQueue.hpp"

namespace dsa {

/**
 * @brief Campus Layout Graph with Shortest Path Navigation (Dijkstra, BFS, DFS).
 * 
 * Time Complexities:
 *  - Dijkstra: O((V + E) log V) using custom Min-Heap
 *  - BFS: O(V + E)
 *  - DFS: O(V + E)
 * 
 * Space Complexity: O(V + E).
 */
class Graph {
public:
    struct Edge {
        int to;
        float weight; // meters or minutes
        std::string description;

        Edge() : to(-1), weight(0.0f) {}
        Edge(int t, float w, std::string d = "") : to(t), weight(w), description(std::move(d)) {}
    };

    struct NodeInfo {
        int id;
        std::string name;
        std::string category; // "Hostel Block", "Mess", "Gate", "Amenity"
    };

    struct PathResult {
        Vector<int> pathNodeIds;
        Vector<std::string> pathNodeNames;
        float totalDistance = 0.0f;
        bool reachable = false;
    };

private:
    Vector<NodeInfo> nodes_;
    Vector<Vector<Edge>> adj_;
    HashTable<std::string, int> nameToId_;

    // Dijkstra Heap Item
    struct PQItem {
        int node;
        float dist;

        bool operator>(const PQItem& other) const {
            return dist > other.dist;
        }
        bool operator<(const PQItem& other) const {
            return dist < other.dist;
        }
    };

    void dfsInternal(int u, Vector<bool>& visited, Vector<int>& result) const {
        visited[u] = true;
        result.push_back(u);
        for (size_t i = 0; i < adj_[u].size(); ++i) {
            int v = adj_[u][i].to;
            if (!visited[v]) {
                dfsInternal(v, visited, result);
            }
        }
    }

public:
    Graph() = default;

    int addNode(const std::string& name, const std::string& category = "Location") {
        int* existing = nameToId_.find(name);
        if (existing) return *existing;

        int id = static_cast<int>(nodes_.size());
        nodes_.push_back({id, name, category});
        adj_.push_back(Vector<Edge>());
        nameToId_.insert(name, id);
        return id;
    }

    void addEdge(int from, int to, float weight, const std::string& desc = "", bool bidirectional = true) {
        if (from >= 0 && static_cast<size_t>(from) < nodes_.size() &&
            to >= 0 && static_cast<size_t>(to) < nodes_.size()) {
            adj_[from].push_back(Edge(to, weight, desc));
            if (bidirectional) {
                adj_[to].push_back(Edge(from, weight, desc));
            }
        }
    }

    void addEdgeByName(const std::string& from, const std::string& to, float weight, const std::string& desc = "") {
        int u = addNode(from);
        int v = addNode(to);
        addEdge(u, v, weight, desc, true);
    }

    int getNodeIndex(const std::string& name) const {
        const int* id = nameToId_.find(name);
        return id ? *id : -1;
    }

    const NodeInfo* getNode(int id) const {
        if (id >= 0 && static_cast<size_t>(id) < nodes_.size()) {
            return &nodes_[id];
        }
        return nullptr;
    }

    [[nodiscard]] size_t nodeCount() const noexcept { return nodes_.size(); }

    // Dijkstra's Algorithm for Shortest Path - O((V + E) log V)
    PathResult shortestPath(int startNode, int endNode) const {
        PathResult res;
        size_t n = nodes_.size();
        if (startNode < 0 || static_cast<size_t>(startNode) >= n ||
            endNode < 0 || static_cast<size_t>(endNode) >= n) {
            return res;
        }

        Vector<float> dist(n, std::numeric_limits<float>::infinity());
        Vector<int> parent(n, -1);
        Vector<bool> visited(n, false);

        MinHeap<PQItem> pq;
        dist[startNode] = 0.0f;
        pq.push({startNode, 0.0f});

        while (!pq.empty()) {
            PQItem top = pq.top();
            pq.pop();
            int u = top.node;

            if (visited[u]) continue;
            visited[u] = true;

            if (u == endNode) break;

            for (size_t i = 0; i < adj_[u].size(); ++i) {
                int v = adj_[u][i].to;
                float weight = adj_[u][i].weight;

                if (!visited[v] && dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    parent[v] = u;
                    pq.push({v, dist[v]});
                }
            }
        }

        if (dist[endNode] == std::numeric_limits<float>::infinity()) {
            return res; // Unreachable
        }

        res.reachable = true;
        res.totalDistance = dist[endNode];

        // Reconstruct path
        Vector<int> revPath;
        for (int at = endNode; at != -1; at = parent[at]) {
            revPath.push_back(at);
        }

        for (int i = static_cast<int>(revPath.size()) - 1; i >= 0; --i) {
            int nodeIdx = revPath[i];
            res.pathNodeIds.push_back(nodeIdx);
            res.pathNodeNames.push_back(nodes_[nodeIdx].name);
        }

        return res;
    }

    // Breadth-First Search - O(V + E)
    Vector<int> bfs(int startNode) const {
        Vector<int> order;
        size_t n = nodes_.size();
        if (startNode < 0 || static_cast<size_t>(startNode) >= n) return order;

        Vector<bool> visited(n, false);
        CircularQueue<int> q(n + 10);

        visited[startNode] = true;
        q.enqueue(startNode);

        while (!q.empty()) {
            int u = q.dequeue();
            order.push_back(u);

            for (size_t i = 0; i < adj_[u].size(); ++i) {
                int v = adj_[u][i].to;
                if (!visited[v]) {
                    visited[v] = true;
                    q.enqueue(v);
                }
            }
        }
        return order;
    }

    // Depth-First Search - O(V + E)
    Vector<int> dfs(int startNode) const {
        Vector<int> order;
        size_t n = nodes_.size();
        if (startNode < 0 || static_cast<size_t>(startNode) >= n) return order;

        Vector<bool> visited(n, false);
        dfsInternal(startNode, visited, order);
        return order;
    }
};

} // namespace dsa
