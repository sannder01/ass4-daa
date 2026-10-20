# Own tests and reproducible evidence

From the repository root, first build with `python tools/run.py --setup`, then run:

```sh
python tools/run.py --check
python tests/verify.py --full
g++ -std=c++17 -O2 tests/route_tests.cpp -o .build/Cpp/route_tests
.build/Cpp/route_tests > tests/route_actual.json
g++ -std=c++17 -O2 tests/benchmark.cpp -o .build/Cpp/benchmark
.build/Cpp/benchmark > tests/benchmark_actual.jsonl
```

The committed `cases.jsonl`, `expected.jsonl` and `actual.jsonl` have matching line numbers. Expected values are generated independently by `verify.py`: explicit DSU relabelling, exhaustive small-forest subsets, mutual reachability closure, Kahn cycle detection, and Bellman-Ford relaxation. Expected witness arrays are examples; semantic checks accept any valid optimal witness.

`verify.py` includes hand-written boundaries and seeded random cases (seed 20261020), plus 338 mixed requests in one process. `witness_tests()` demonstrates valid alternative and invalid witnesses for both forests, SCC, topo and DAG. `results.json` records actual pass results and full-limit output hashes.

`scale_cases()` generates eight cases at n=100000 and 250000 rows without committing huge repetitive inputs: chain unions and repeats; connected unit-weight forests; a forward SCC/topo chain and a chain with a closing edge; and a weighted DAG chain. It computes expected values from construction and validates complete outputs. The forward chain has singleton SCCs, forced ascending topological order and distance `i*1000000000`. The closing edge makes the whole chain strongly connected. A connected unit-weight spanning tree costs n-1. Chain DSU unions reduce components by one and increase largest by one; later repeated connections change nothing.

`route_tests.cpp` calls the actual C++ helper directly, checks directed edges and route costs, the source, no route, tied/parallel edges, and a 100000-vertex route. `route_actual.json` contains its actual output.

`benchmark.cpp` uses the same generated input for both methods at three sizes with one warm-up and three measurements each. Its generated negative-weight chain is the unique optimum because all extra edges have nonnegative weights. Selected IDs are checked before and after timing. `benchmark_actual.jsonl` records all observations. See `../ANALYSIS.md` for timing boundaries, environment, medians and limitations.

## Original starter guidance

Create your own JSON Lines inputs here, together with expected and actual outputs and an explanation of each test. Run a file from the repository root:

```sh
python tools/run.py --input tests/my_cases.jsonl
```

Create `my_cases.jsonl` before running this example command. The launcher prints actual results; compare them with independently justified expected answers. Your files are not discovered automatically. Call your route helper directly in your test code. See the root README for all requirements.

Создайте здесь свои входы JSON Lines, ожидаемые и фактические результаты и объяснения тестов. Команда выше запускает созданный вами файл, но не сравнивает ответы автоматически. Функцию восстановления пути вызывайте непосредственно из своего тестового кода. Полные требования приведены в README в корне репозитория.

