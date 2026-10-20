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

    throw runtime_error("TODO kruskal");
}

// Separate Prim implementation; restart for every unvisited component.
inline J prim(const J& input) {
    int n = Input::vertexCount(input);
    vector<vector<int>> edges = Input::edges(input);

    throw runtime_error("TODO prim");
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
