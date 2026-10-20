#pragma once
#include "input.hpp"

// Implement the six graph methods and your route helper. See the README at the repository root.
#include <iostream>
#include <vector>
#include <utility>
using namespace std;
//находит корень - как? prosto vse smotrit
int findroot(vector<int>& parent, int x) {
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
bool unit(vector<int>& parent, vector<int>& sz, int a, int b) {
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
        //здесь если у вершины две еджес мы вес прибавляем и компоненты отнимаем
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


// Kosaraju with two DFS passes; return component and condensation.
inline J scc(const J& input) {
    int n = Input::vertexCount(input);
    vector<vector<int>> edges = Input::edges(input);

    throw runtime_error("TODO scc");
}

// Three-state DFS and reverse finish order; return acyclic and order.
inline J topo(const J& input) {
    int n = Input::vertexCount(input);
    vector<vector<int>> edges = Input::edges(input);

    throw runtime_error("TODO topo");
}

// Whole-graph cycle check and DAG relaxation; add your own route helper.
inline J dag(const J& input) {
    int n = Input::vertexCount(input);
    int source = Input::source(input);
    vector<vector<int>> edges = Input::edges(input);

    throw runtime_error("TODO dag");
}
