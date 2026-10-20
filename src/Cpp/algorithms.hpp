#pragma once
#include "input.hpp"

// Implement the six graph methods and your route helper. See the README at the repository root.
#include <iostream>
#include <vector>
#include <utility>
using namespace std;
// Идём по родителям до вершины, которая указывает на себя.
inline int findroot(vector<int>& parent, int x) {
    int root = x;

    while (parent[root] != root) {
        root = parent[root];
    }

    int u = x;
    //это я взял из практика урока когда вы объясняли ну я понял короче это сжатие 
    while (u != root) {
        int next = parent[u];
        parent[u] = root;
        u = next;
    }

    return root;
}
//это у нас объеденение унит мы тут объеденяем 
inline bool unit(vector<int>& parent, vector<int>& sz, int a, int b) {
    int rootA = findroot(parent, a);
    int rootB = findroot(parent, b);

    if (rootA == rootB) {
        return false;
    }

    if (sz[rootA] < sz[rootB]) {
        swap(rootA, rootB);
    }

    parent[rootB] = rootA;
    sz[rootA] += sz[rootB];

    return true;
}

// DSU with path compression and union by size/rank; return states.
inline J dsu(const J& input) {
    int n = Input::vertexCount(input);
    vector<vector<int>> operations = Input::operations(input);

    vector<int> parent(n);
    vector<int> sz(n, 1);

    for (int i = 0; i < n; i++) {
        parent[i] = i;
    }

    int components = n;
    int largest = (n == 0 ? 0 : 1);

    J::A states;

    for (const auto& op : operations) {
        int a = op[0];
        int b = op[1];

        if (unit(parent, sz, a, b)) {
            components--;

            int root = findroot(parent, a);

            if (sz[root] > largest) {
                largest = sz[root];
            }
        }

        states.push_back(J::A{components, largest});
    }

    J result;
    result["states"] = states;
    return result;
}

// Separate Kruskal implementation; return cost, edge_ids, components.
inline J kruskal(const J& input) {
    int n = Input::vertexCount(input);
    vector<vector<int>> edges = Input::edges(input);

    vector<int> parent(n);
    vector<int> sz(n, 1);

    for (int i = 0; i < n; i++) {
        parent[i] = i;
    }

    //вектор вектор как в матрице я назыаю такое секрет виктора
    vector<vector<int>> sorted;
    
    for (int i = 0; i < edges.size(); i++) {
        int weight = edges[i][2];
        sorted.push_back({weight, i});
    }

    // сортируем
    sort(sorted.begin(), sorted.end());

    long long cost = 0;
    int components = n;
    J::A chosen;

    for (int i = 0; i < sorted.size(); i++) {
        int id = sorted[i][1];

        int a = edges[id][0];
        int b = edges[id][1];
        int weight = edges[id][2];
        // Берём ребро, только если оно соединяет разные множества.
        if (unit(parent, sz, a, b)) {
            cost += weight;
            components--;
            chosen.push_back(id);
        }
    }

    J result;
    result["cost"] = cost;
    result["edge_ids"] = chosen;
    result["components"] = components;
    return result;
}
// Separate Prim implementation; restart for every unvisited component.
inline J prim(const J& input) {
    int n = Input::vertexCount(input);
    vector<vector<int>> edges = Input::edges(input);

    vector<vector<int>> graph(n);

    for (int i = 0; i < edges.size(); i++) {
        int a = edges[i][0];
        int b = edges[i][1];

        graph[a].push_back(i);
        graph[b].push_back(i);
    }

    vector<bool> visited(n, false);
    long long cost = 0;
    int components = 0;
    J::A chosen;

    using Item = pair<int, int>;
    priority_queue<Item, vector<Item>, greater<Item>> q;

    for (int start = 0; start < n; start++) {
        if (visited[start]) {
            continue;
        }

        components++;
        visited[start] = true;

        for (int i = 0; i < graph[start].size(); i++) {
            int id = graph[start][i];
            int weight = edges[id][2];
            q.push({weight, id});
        }

        while (!q.empty()) {
            int id = q.top().second;
            q.pop();

            int a = edges[id][0];
            int b = edges[id][1];
            int weight = edges[id][2];

            if (visited[a] && visited[b]) {
                continue;
            }

            int next = a;

            if (visited[a]) {
                next = b;
            }

            visited[next] = true;
            cost += weight;
            chosen.push_back(id);

            for (int i = 0; i < graph[next].size(); i++) {
                int edgeId = graph[next][i];
                int edgeWeight = edges[edgeId][2];
                q.push({edgeWeight, edgeId});
            }
        }
    }

    J result;
    result["cost"] = cost;
    result["edge_ids"] = chosen;
    result["components"] = components;
    return result;
}


// color: 0 = не заходили, 1 = сейчас в стеке, 2 = закончили.
// nextEdge хранит, какого соседа будем смотреть следующим.
inline vector<int> dfsFinishOrder(const vector<vector<int>>& graph, bool& acyclic) {
    int n = graph.size();
    vector<int> color(n, 0);
    vector<int> nextEdge(n, 0);
    vector<int> path;
    vector<int> order;
    acyclic = true;

    for (int start = 0; start < n; start++) {
        if (color[start] != 0) {
            continue;
        }

        color[start] = 1;
        path.push_back(start);

        while (!path.empty()) {
            int vertex = path.back();

            if (nextEdge[vertex] == graph[vertex].size()) {
                color[vertex] = 2;
                order.push_back(vertex);
                path.pop_back();
                continue;
            }

            int next = graph[vertex][nextEdge[vertex]];
            nextEdge[vertex]++;

            if (color[next] == 1) {
                acyclic = false;
            }

            if (color[next] == 0) {
                color[next] = 1;
                path.push_back(next);
            }
        }
    }

    return order;
}

// Kosaraju with two DFS passes; return component and condensation.
inline J scc(const J& input) {
    int n = Input::vertexCount(input);
    vector<vector<int>> edges = Input::edges(input);

    vector<vector<int>> graph(n);
    vector<vector<int>> reversed(n);

    for (int i = 0; i < edges.size(); i++) {
        int a = edges[i][0];
        int b = edges[i][1];
        graph[a].push_back(b);
        reversed[b].push_back(a);
    }

    bool acyclic;
    vector<int> order = dfsFinishOrder(graph, acyclic);
    reverse(order.begin(), order.end());

    vector<int> component(n, -1);
    int label = 0;

    for (int i = 0; i < order.size(); i++) {
        int start = order[i];
        if (component[start] != -1) {
            continue;
        }

        vector<int> pending;
        pending.push_back(start);
        component[start] = label;

        while (!pending.empty()) {
            int vertex = pending.back();
            pending.pop_back();

            for (int j = 0; j < reversed[vertex].size(); j++) {
                int next = reversed[vertex][j];

                if (component[next] == -1) {
                    component[next] = label;
                    pending.push_back(next);
                }
            }
        }

        label++;
    }

    J::A labels;
    for (int i = 0; i < n; i++) {
        labels.push_back(component[i]);
    }

    J::A condensation;
    unordered_set<long long> seen;

    for (int i = 0; i < edges.size(); i++) {
        int from = component[edges[i][0]];
        int to = component[edges[i][1]];

        if (from == to) {
            continue;
        }

        long long key = 1LL * from * n + to;
        if (seen.count(key) == 0) {
            seen.insert(key);
            condensation.push_back(J::A{from, to});
        }
    }

    J result;
    result["component"] = labels;
    result["condensation"] = condensation;
    return result;
}

// Three-state DFS and reverse finish order; return acyclic and order.
inline J topo(const J& input) {
    int n = Input::vertexCount(input);
    vector<vector<int>> edges = Input::edges(input);

    vector<vector<int>> graph(n);

    for (int i = 0; i < edges.size(); i++) {
        int a = edges[i][0];
        int b = edges[i][1];
        graph[a].push_back(b);
    }

    bool acyclic;
    vector<int> order = dfsFinishOrder(graph, acyclic);
    J::A answer;

    if (acyclic) {
        reverse(order.begin(), order.end());
        for (int i = 0; i < order.size(); i++) {
            answer.push_back(order[i]);
        }
    }

    J result;
    result["acyclic"] = acyclic;
    result["order"] = answer;
    return result;
}

// Whole-graph cycle check and DAG relaxation; add your own route helper.
inline J dag(const J& input) {
    int n = Input::vertexCount(input);
    int source = Input::source(input);
    vector<vector<int>> edges = Input::edges(input);

    vector<vector<int>> graph(n);
    vector<vector<int>> outgoing(n);

    for (int i = 0; i < edges.size(); i++) {
        int a = edges[i][0];
        int b = edges[i][1];
        graph[a].push_back(b);
        outgoing[a].push_back(i);
    }

    bool acyclic;
    vector<int> order = dfsFinishOrder(graph, acyclic);
    J result;
    result["acyclic"] = acyclic;

    if (!acyclic) {
        result["distance"] = J::A{};
        result["parent_edge"] = J::A{};
        return result;
    }

    reverse(order.begin(), order.end());

    vector<long long> distance(n, 0);
    vector<bool> reachable(n, false);
    vector<int> parentEdge(n, -1);
    reachable[source] = true;

    for (int i = 0; i < order.size(); i++) {
        int vertex = order[i];

        if (!reachable[vertex]) {
            continue;
        }

        for (int j = 0; j < outgoing[vertex].size(); j++) {
            int id = outgoing[vertex][j];
            int next = edges[id][1];
            int weight = edges[id][2];
            long long candidate = distance[vertex] + weight;

            if (!reachable[next] || candidate < distance[next]) {
                reachable[next] = true;
                distance[next] = candidate;
                parentEdge[next] = id;
            }
        }
    }

    J::A distances;
    J::A parents;

    for (int i = 0; i < n; i++) {
        if (reachable[i]) {
            distances.push_back(distance[i]);
        } else {
            distances.push_back(J());
        }
        parents.push_back(parentEdge[i]);
    }

    result["distance"] = distances;
    result["parent_edge"] = parents;
    return result;
}

// Для корректного результата dag: идём от цели по входящим рёбрам назад.
inline vector<int> reconstructRoute(int source, int target,
                                   const vector<int>& parentEdge,
                                   const vector<vector<int>>& edges) {
    vector<int> route;
    int vertex = target;

    while (vertex != source) {
        int id = parentEdge[vertex];
        if (id == -1) {
            return {};
        }

        route.push_back(vertex);
        vertex = edges[id][0];
    }

    route.push_back(source);
    reverse(route.begin(), route.end());
    return route;
}

