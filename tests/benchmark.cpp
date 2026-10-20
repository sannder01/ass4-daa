#include "../src/Cpp/algorithms.hpp"
#include <chrono>

void verifyForest(const J& result, int n, long long expectedCost) {
    if (result["cost"].number() != expectedCost || result["components"].number() != 1) {
        throw runtime_error("Wrong forest cost or components");
    }
    vector<bool> seen(n - 1, false);
    const J::A& ids = result["edge_ids"].array();
    if (ids.size() != n - 1) throw runtime_error("Wrong forest size");
    for (const J& value : ids) {
        int id = value.number();
        if (id < 0 || id >= n - 1 || seen[id]) throw runtime_error("Invalid forest witness");
        seen[id] = true;
    }
}

int main() {
    vector<int> sizes = {100, 5000, 50000};
    J::A all;
    for (int n : sizes) {
        int seed = 20261020 + n;
        mt19937 random(seed);
        int m = 3 * n;
        J::A edges;
        long long expectedCost = 0;
        for (int i = 1; i < n; i++) {
            int weight = -1000000000 + static_cast<int>(random() % 1000000);
            edges.push_back(J::A{i - 1, i, weight});
            expectedCost += weight;
        }
        while (edges.size() < m) {
            int a = random() % n;
            int b = random() % n;
            int weight = random() % 1000000;
            edges.push_back(J::A{a, b, weight});
        }

        J request;
        request["n"] = n;
        request["edges"] = edges;
        verifyForest(kruskal(request), n, expectedCost);
        verifyForest(prim(request), n, expectedCost);

        vector<vector<double>> timings(2);
        for (int trial = 0; trial < 3; trial++) {
            for (int step = 0; step < 2; step++) {
                int method = (step + trial) % 2;
                auto begin = chrono::steady_clock::now();
                J answer = method == 0 ? kruskal(request) : prim(request);
                auto end = chrono::steady_clock::now();
                timings[method].push_back(chrono::duration<double, milli>(end - begin).count());
                verifyForest(answer, n, expectedCost);
            }
        }

        for (int method = 0; method < 2; method++) {
            vector<double> sorted = timings[method];
            sort(sorted.begin(), sorted.end());
            cout << "{\"method\":\"" << (method == 0 ? "kruskal" : "prim")
                 << "\",\"n\":" << n << ",\"m\":" << m << ",\"seed\":" << seed
                 << ",\"cost\":" << expectedCost << ",\"milliseconds\":[";
            for (int i = 0; i < 3; i++) {
                if (i > 0) cout << ',';
                cout << fixed << setprecision(6) << timings[method][i];
            }
            cout << "],\"median_ms\":" << sorted[1] << "}\n";
        }
    }
}
