# Graph algorithms analysis
# вы не представляете как я устал при выполнении данного задания


The C++ solution implements all six required methods and a vertex-route helper. The supplied JSON Lines transport and input adapters are unchanged. Original edge IDs are preserved. All cost and distance sums use `long long`.

## Build and run

Run from the repository root with Python 3.12+ and a C++17 compiler:

```sh
python tools/run.py --setup
python tools/run.py --check
python tools/run.py --input tests/cases.jsonl
python tests/verify.py --full
g++ -std=c++17 -O2 tests/route_tests.cpp -o .build/Cpp/route_tests
.build/Cpp/route_tests
g++ -std=c++17 -O2 tests/benchmark.cpp -o .build/Cpp/benchmark
.build/Cpp/benchmark
```

The setup command creates `.build/Cpp`. A direct application build is:

```sh
g++ -std=c++17 -O2 src/Cpp/main.cpp -o .build/Cpp/main
.build/Cpp/main < tests/cases.jsonl
```

## DSU

Each vertex starts as its own parent with size one. `findroot` first follows parent links to a self-parent, then traverses the same path and points each visited vertex directly at that root. `unit` finds both roots, returns false when they coincide, and otherwise attaches the smaller component to the larger one and adds their sizes.

Invariant: parent pointers form rooted trees; a root's size is exactly the number of vertices in that tree. Finding and compressing a path preserve the partition. Linking two different roots joins exactly those two components, so decreasing the component count once is correct. Non-root size entries are not used. The maximum size only needs updating after a successful union.

An output state is appended after every operation, including repeated unions and self-unions. With no vertices there are no operations or states; the initial largest size is zero. Other initial largest sizes are one. For `q` operations, time is `O(n + q alpha(n))` amortized and space is `O(n+q)` including returned states and converted operations.

## Kruskal

The sorted list stores `[weight, original edge ID]`. Edges are processed in nondecreasing weight order. A successful DSU union selects the edge, adds its weight and decreases the component count. Self-loops and redundant edges cannot join different roots.

Invariant: selected edges form a forest which can be extended to a minimum spanning forest. A selected edge is a lightest edge crossing a cut between current DSU components. The cut/exchange property makes this edge safe. When all edges have been examined, each input component is connected: an edge between remaining different components would have been accepted. The forest therefore has exactly `n-components` edges and minimum cost.

Negative and tied weights do not change this argument. Parallel edges retain separate IDs. Isolates stay as single components; empty input returns zero cost, zero components and no IDs. Sorting takes `O(m log(m+1))`; initialization and DSU give total `O(n + m log(m+1))` time and `O(n+m)` space.

## Prim

Adjacency lists contain edge IDs. Start from an unvisited vertex, mark it, and insert its incident edges into a min-priority queue of `[weight, ID]` pairs. Remove the cheapest edge. If both endpoints are already visited, discard it. Otherwise attach the unvisited endpoint and insert its incident edges. Restart from each remaining unvisited vertex.

Invariant within one search: selected edges form a tree on the visited vertices of that input component, and every edge crossing from that tree to an unvisited vertex is in the queue. An inserted edge always has a visited endpoint. Thus an edge not discarded by the two-visited-endpoints check has exactly one new endpoint. The cheapest remaining crossing edge is safe by the cut property. Exhausting the queue completes the component; restarting constructs a minimum forest. Every restart, including at an isolate, adds one component.

The queue may contain stale entries and parallel edges; discarding them is intentional. A self-loop is stale immediately. Negative weights are supported by the same min-heap comparison. Empty input performs no restarts. Each undirected edge is inserted at most twice, so the lazy-heap time is `O(n + m log(m+1))`, with `O(n+m)` space.

## Iterative DFS and deep graphs

`dfsFinishOrder` uses three colors: zero is unvisited, one is active, and two is finished. `path` is an explicit stack of active vertices. `nextEdge[v]` stores the position of the next neighbour to examine when returning to vertex `v`. A vertex is appended to the finishing list only after all neighbours have been processed.

An edge to an active vertex closes a path back to an ancestor and therefore proves a directed cycle. A finished neighbour does not prove a cycle. The helper continues after finding a cycle because SCC still requires a complete finishing list. Its Boolean output is used by `topo` and `dag`.

Each vertex enters and leaves the stack once; each adjacency entry is examined once. Remembering the next-neighbour position prevents rescanning a high-degree vertex after every child. Time is `O(n+m)` and extra space is `O(n)`. No recursive calls are used, including in the SCC second pass and route helper. A chain of 100000 vertices therefore does not consume the native call stack.

## Kosaraju SCC

Build the graph and its transpose. Obtain first-pass DFS finish order, reverse it, then traverse the transpose from each still-unlabelled vertex in that order. Mark vertices when pushing them onto the stack and assign the same label throughout one traversal. Increase the label for the next traversal.

In the condensation DAG, an edge from one SCC to another gives the source SCC a larger first-pass finishing maximum. Processing decreasing finish times on the transpose therefore starts from a remaining SCC with no outgoing transpose edge to another unlabelled SCC. Mutual reachability ensures the whole current SCC is reached, and no other unlabelled SCC is included. By induction every label identifies exactly one maximal strongly connected component.

Scan the original edges to form condensation edges. Ignore equal labels and insert each cross-component pair once. The hash key `from*n+to` is unique because labels are in `[0,n)`, and it is calculated as `long long`. `unordered_set` gives expected constant-time deduplication.

Isolates get distinct labels. Loops remain inside their SCC; duplicate cross-edges yield only one pair. Empty input yields two empty arrays. Expected time is `O(n+m)` and memory is `O(n+m)`; hash-table worst-case operations are not claimed to be constant time.

## DFS topological ordering

Build directed adjacency lists and run the three-color helper from every unvisited vertex. If it observes any edge to an active vertex, return `acyclic=false` and an empty order. Otherwise reverse the finishing list.

For every edge `u -> v` of an acyclic graph, `v` finishes before `u`: either DFS visits it from `u`, or it was already finished. An active `v` would indicate a cycle and has been excluded. Reversing finish order therefore puts `u` before `v`. Each vertex is appended exactly once, so the result is a permutation of all vertices. A self-loop is detected as a cycle; disconnected cycles are also visited. Empty input is acyclic with an empty order. Time and total memory are `O(n+m)`.

## DAG shortest paths

First perform the same whole-graph cycle check and topological ordering, including components unreachable from the source. A cyclic graph returns `false` and two empty arrays. For an acyclic graph, mark only the source reachable and initialize every parent edge to `-1`. Process vertices in topological order and skip unreachable vertices. For each outgoing edge, update the destination if this is its first reachable path or the candidate distance is strictly smaller.

Invariant: when a reachable vertex is processed, its distance is final. Every incoming predecessor is earlier in the topological order, so all possible final edges of a path to it have already been relaxed. This proves shortest distances by induction, even for negative weights. Equal-cost candidates keep the earlier witness. Every stored parent is an original incoming edge on a path of the reported cost; its predecessor is earlier in the order, so parent chains reach the source without cycling.

Reachability is stored separately from distances: zero and minus one are both valid finite distances. Unreachable JSON distances are `null`; their parents and the source parent are `-1`. Weighted path length is at most `99999 * 1000000000` in magnitude, safely inside signed 64 bits. Parallel edges remain distinct. Time and total space are `O(n+m)`.

## Route helper

`reconstructRoute(source, target, parentEdge, edges)` takes the parent IDs from a successful acyclic DAG result for that source. These valid-result inputs are its precondition; it is not a validator for arbitrary or cyclic parent arrays. Starting at the target, append the vertex and follow the chosen incoming edge back to its origin. A missing parent before reaching the source means no route and returns an empty vector. Append the source and reverse the vector. Time and output space are `O(L)` for a route of `L` vertices. The route to the source is `[source]`.

Actual direct calls in `tests/route_tests.cpp` use edges with original IDs:

| ID | Edge |
|---|---|
| 0 | 0 -> 1, weight 3 |
| 1 | 0 -> 2, weight 8 |
| 2 | 1 -> 2, weight -7 |
| 3 | 2 -> 3, weight 4 |
| 4 | 0 -> 1, weight 3 |
| 5 | 0 -> 2, weight -4 |

The DAG result chooses parents `[-1,0,5,3,-1]`. Direct helper results saved in `tests/route_actual.json`:

| Call | Vertices | Selected edge IDs | Cost |
|---|---|---|---|
| `reconstructRoute(0,3,parents,edges)` | `[0,2,3]` | `[5,3]` | 0 |
| `reconstructRoute(0,0,parents,edges)` | `[0]` | `[]` | 0 |
| `reconstructRoute(0,4,parents,edges)` | `[]` | `[]` | no route |
| `reconstructRoute(0,1,parents,edges)` | `[0,1]` | `[0]` | 3 |
| Same call after `parents[1]=4` | `[0,1]` | `[4]` | 3 |

The graph also has tied paths to vertex 2: edge 5 costs -4, as do edges 0 and 2 together. The direct route and its direction and total weight are checked. A separate helper call reconstructs and checks every vertex of a 100000-vertex route.

## State reset and test evidence

Every graph, array, queue, hash set and result is local to a function call. No state is static or global. The supplied entry point preserves `ping` and prints only one JSON result per request.

Observed checks in the execution environment:

- `--setup`: ping passed.
- `--check`: all 36 public cases passed, plus two full batches in one process.
- `python tests/verify.py --full`: 169 own small cases passed; all 338 requests in a repeated mixed batch returned the same answers; five semantic-witness demonstrations accepted alternative valid answers and rejected invalid ones.
- Eight scale cases passed with `n=100000` and exactly 250000 edges or operations: DSU, both forests, SCC chain, SCC cycle, topological chain, topological cycle and DAG chain. These include a DFS depth of 100000 and a final DAG distance of 99999000000000.
- Direct route tests passed, including the 100000-vertex route.

`tests/cases.jsonl`, `expected.jsonl` and `actual.jsonl` store the small inputs and results. `verify.py` provides their independent justification: explicit relabelling for DSU, exhaustive spanning subsets for small forests, transitive closure for SCC, Kahn's algorithm for cycle existence, and repeated Bellman-Ford relaxation for DAG distances. The supplied semantic validator checks the actual forest IDs, exact condensation pairs, precedence and parent chains. Random cases use seed 20261020, and hand-written cases include boundaries, loops, duplicates, ties, isolates, negative weights, 64-bit totals and cycles unreachable from the source.

Scale inputs are generated by `scale_cases()` rather than storing many megabytes of repeated data. Expected results follow directly from their construction: a unit-weight connected graph needs `n-1` forest edges; a forward chain has singleton SCCs and forced ascending topological order; adding a return edge makes all chain vertices one SCC and creates a cycle; a weighted forward chain has distance `i*1000000000`. DSU performs chain unions followed by redundant connections. `tests/results.json` records actual large-output sizes, SHA-256 hashes and selected result values, plus all pass results. Rerunning the script regenerates and validates the complete outputs in memory.

## Kruskal and Prim benchmark

Compiled separately with `g++ -std=c++17 -O2 tests/benchmark.cpp -o .build/Cpp/benchmark`; run `.build/Cpp/benchmark`. Raw observations are in `tests/benchmark_actual.jsonl`.

Environment: Linux 6.18.55 x86_64 in a shared KVM container; Intel Xeon Platinum 8573C; 9 visible logical CPUs; g++ 13.3.0; Python 3.12.14 for the checking scripts. These measurements describe this environment, not the student's computer.

Generator: `std::mt19937` with seed `20261020+n`, `m=3*n`. First add a chain with weights `-1000000000 + rng()%1000000`; add uniformly modulo-selected endpoint pairs with weights `rng()%1000000` until reaching `m`. Both algorithms receive the exact same prebuilt request at each size. The chain consists of the `n-1` strictly cheapest edges and already spans the graph, so it is the unique optimum. Before timing and after each trial the benchmark verifies the optimum cost, component count and that every chain edge ID is selected exactly once.

One untimed invocation per method and size provides warm-up. There are three timed trials per method per size, alternating which method runs first. `std::chrono::steady_clock` measures milliseconds immediately around the algorithm call, including `Input::edges` conversion, algorithm data structures and return-object construction. Request generation, initial JSON object construction, parsing text, serialization, stdout, result validation, destruction of the returned object, and compilation are excluded. Both methods use the same boundary.

| n | m | Method | Three times in ms | Median ms |
|---|---|---|---|---|
| 100 | 300 | kruskal | 0.354900, 0.143655, 0.036409 | 0.143655 |
| 100 | 300 | prim | 0.330476, 2.030347, 0.075056 | 0.330476 |
| 5000 | 15000 | kruskal | 26.731714, 14.010552, 17.652507 | 17.652507 |
| 5000 | 15000 | prim | 38.009933, 27.378179, 54.799619 | 38.009933 |
| 50000 | 150000 | kruskal | 287.373065, 209.359645, 240.597067 | 240.597067 |
| 50000 | 150000 | prim | 578.145563, 465.703265, 446.679492 | 465.703265 |

On these measured graphs Kruskal had lower median times at all three sizes. Sorting a compact list and union operations were cheaper here than building adjacency lists and managing Prim's lazy heap. Small measurements vary substantially due to scheduling and timer noise. Three trials, one graph family and a shared machine are limited evidence; these timings do not establish universal superiority or replace the asymptotic argument. Both implemented forest algorithms have the same published asymptotic upper bound for this lazy-heap version of Prim.

## Sources and AI assistance

помог мне код объяснить и аналис сделать ну в плане код я писал и ему просил помочь чут чуть
