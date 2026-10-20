#include "../src/Cpp/algorithms.hpp"

int main() {
    J request = J::parse(R"({"task":"dag","n":5,"source":0,"edges":[[0,1,3],[0,2,8],[1,2,-7],[2,3,4],[0,1,3],[0,2,-4]]})");
    J answer = dag(request);
    vector<vector<int>> edges = Input::edges(request);
    vector<int> parents;
    for (const J& id : answer["parent_edge"].array()) {
        parents.push_back(id.number());
    }

    vector<vector<int>> expected = {{0, 2, 3}, {0}, {}};
    vector<int> targets = {3, 0, 4};
    J::A results;

    for (int i = 0; i < targets.size(); i++) {
        int target = targets[i];
        vector<int> route = reconstructRoute(0, target, parents, edges);
        if (route != expected[i]) return 1;
        J::A vertices;
        J::A ids;
        long long cost = 0;
        for (int j = 0; j < route.size(); j++) {
            vertices.push_back(route[j]);
            if (j > 0) {
                int id = parents[route[j]];
                if (edges[id][0] != route[j - 1] || edges[id][1] != route[j]) return 2;
                ids.push_back(id);
                cost += edges[id][2];
            }
        }
        results.push_back(J::O{{"target", target}, {"vertices", vertices},
                               {"edge_ids", ids}, {"cost", route.empty() ? J() : J(cost)}});
    }

    vector<int> route1 = reconstructRoute(0, 1, parents, edges);
    if (route1 != vector<int>({0, 1}) || parents[1] != 0) return 3;
    parents[1] = 4;
    vector<int> route2 = reconstructRoute(0, 1, parents, edges);
    if (route1 != route2) return 4;
    results.push_back(J::O{{"target", 1}, {"vertices", J::A{0, 1}},
                           {"parallel_edge_ids_tested", J::A{0, 4}}, {"cost", 3}});

    vector<int> deepParents(100000, -1);
    vector<vector<int>> deepEdges;
    for (int i = 1; i < 100000; i++) {
        deepEdges.push_back({i - 1, i, 1});
        deepParents[i] = i - 1;
    }
    vector<int> deepRoute = reconstructRoute(0, 99999, deepParents, deepEdges);
    if (deepRoute.size() != 100000) return 5;
    for (int i = 0; i < deepRoute.size(); i++) if (deepRoute[i] != i) return 6;

    J result;
    result["calls"] = results;
    result["deep_route_vertices"] = static_cast<int>(deepRoute.size());
    cout << result.dump() << '\n';
}
