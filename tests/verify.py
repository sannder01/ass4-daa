"""Independent small references, semantic witnesses and full-limit checks."""
import argparse
import copy
import hashlib
import itertools
import json
from pathlib import Path
import random
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from public_validate import check


def run(requests):
    text = ''.join(json.dumps(x, separators=(',', ':')) + '\n' for x in requests)
    binary = ROOT / '.build/Cpp/main'
    result = subprocess.run([str(binary)], input=text, text=True,
                            capture_output=True, check=True, timeout=20)
    answers = [json.loads(line) for line in result.stdout.splitlines()]
    assert len(answers) == len(requests), 'One result per request required'
    return answers


def reachability(n, edges, undirected=False):
    reach = [[i == j for j in range(n)] for i in range(n)]
    for a, b, *_ in edges:
        reach[a][b] = True
        if undirected:
            reach[b][a] = True
    for k in range(n):
        for i in range(n):
            for j in range(n):
                reach[i][j] |= reach[i][k] and reach[k][j]
    return reach


def reference(x):
    task, n = x['task'], x['n']
    if task == 'dsu':
        labels = list(range(n))
        states = []
        for a, b in x['operations']:
            old, new = labels[b], labels[a]
            labels = [new if v == old else v for v in labels]
            states.append([len(set(labels)), max(labels.count(v) for v in set(labels))])
        return {'states': states}
    edges = x['edges']
    if task in ('kruskal', 'prim'):
        reach = reachability(n, edges, True)
        components = len({tuple(row) for row in reach})
        best = None
        best_ids = []
        # Exhaustive subsets, not another implementation of Kruskal/Prim.
        for ids in itertools.combinations(range(len(edges)), n - components):
            selected = [edges[i] for i in ids]
            if reachability(n, selected, True) == reach:
                cost = sum(row[2] for row in selected)
                if best is None or cost < best:
                    best, best_ids = cost, list(ids)
        return {'cost': best, 'components': components, 'edge_ids': best_ids}
    if task == 'scc':
        reach = reachability(n, edges)
        labels = [min(j for j in range(n) if reach[i][j] and reach[j][i]) for i in range(n)]
        pairs = sorted({(labels[a], labels[b]) for a, b in edges if labels[a] != labels[b]})
        return {'component': labels, 'condensation': [list(p) for p in pairs]}
    # Independent Kahn cycle test, rather than the submitted DFS.
    degree = [0] * n
    graph = [[] for _ in range(n)]
    for a, b, *_ in edges:
        graph[a].append(b)
        degree[b] += 1
    order = [v for v in range(n) if degree[v] == 0]
    for a in order:
        for b in graph[a]:
            degree[b] -= 1
            if degree[b] == 0:
                order.append(b)
    acyclic = len(order) == n
    if task == 'topo':
        return {'acyclic': acyclic, 'order': order if acyclic else []}
    if not acyclic:
        return {'acyclic': False, 'distance': [], 'parent_edge': []}
    distance, parents = [None] * n, [-1] * n
    distance[x['source']] = 0
    # Independent repeated Bellman-Ford relaxation on small DAGs.
    for _ in range(n - 1):
        for i, (a, b, w) in enumerate(edges):
            if distance[a] is not None and (distance[b] is None or distance[a] + w < distance[b]):
                distance[b], parents[b] = distance[a] + w, i
    return {'acyclic': True, 'distance': distance, 'parent_edge': parents}


def small_cases():
    cases = []
    for task in ('dsu', 'kruskal', 'prim', 'scc', 'topo'):
        cases.append({'task': task, 'n': 0, 'operations' if task == 'dsu' else 'edges': []})
    cases += [
        {'task': 'dsu', 'n': 4, 'operations': [[0, 0], [0, 1], [1, 0], [2, 3], [1, 3]]},
        {'task': 'dag', 'n': 1, 'source': 0, 'edges': []},
        {'task': 'dag', 'n': 4, 'source': 0, 'edges': [[0, 1, -1], [2, 3, 0], [3, 2, 0]]},
        {'task': 'dag', 'n': 5, 'source': 0, 'edges': [[0, 1, 3], [0, 1, 3], [1, 2, -7], [0, 2, -4]]},
        {'task': 'dag', 'n': 4, 'source': 0, 'edges': [[0, 1, 1000000000], [1, 2, 1000000000], [2, 3, 1000000000]]},
        {'task': 'dag', 'n': 2, 'source': 1, 'edges': [[0, 1, -5]]},
        {'task': 'dag', 'n': 1, 'source': 0, 'edges': [[0, 0, -1]]},
        {'task': 'scc', 'n': 5, 'edges': [[0, 1], [1, 0], [1, 2], [0, 2], [2, 2], [2, 3]]},
        {'task': 'topo', 'n': 4, 'edges': [[0, 2], [1, 2], [0, 2], [2, 3]]},
        {'task': 'topo', 'n': 4, 'edges': [[0, 1], [2, 3], [3, 2]]},
    ]
    for task in ('kruskal', 'prim'):
        cases.append({'task': task, 'n': 5, 'edges': [[0, 0, -100], [0, 1, -2], [0, 1, -2], [1, 2, 3], [0, 2, 8]]})
        cases.append({'task': task, 'n': 4, 'edges': [[0, 1, 1000000000], [1, 2, 1000000000], [2, 3, 1000000000]]})
    rng = random.Random(20261020)
    for task in ('dsu', 'kruskal', 'prim', 'scc', 'topo', 'dag'):
        for _ in range(25):
            n = rng.randint(1, 6)
            x = {'task': task, 'n': n}
            if task == 'dsu':
                x['operations'] = [[rng.randrange(n), rng.randrange(n)] for _ in range(10)]
            else:
                edges = []
                for _ in range(9):
                    a, b = rng.randrange(n), rng.randrange(n)
                    if task == 'dag' and a >= b:
                        continue
                    row = [a, b]
                    if task in ('kruskal', 'prim', 'dag'):
                        row.append(rng.randint(-5, 5))
                    edges.append(row)
                x['edges'] = edges
                if task == 'dag':
                    x['source'] = rng.randrange(n)
            cases.append(x)
    return cases


def witness_tests():
    examples = [
        ({'task': 'kruskal', 'n': 3, 'edges': [[0, 1, 1], [1, 2, 1], [0, 2, 1]]},
         {'cost': 2, 'components': 1, 'edge_ids': [0, 1]},
         {'cost': 2, 'components': 1, 'edge_ids': [1, 2]},
         {'cost': 2, 'components': 1, 'edge_ids': [0, 0]}),
        ({'task': 'scc', 'n': 3, 'edges': [[0, 1], [1, 0], [1, 2]]},
         {'component': [0, 0, 1], 'condensation': [[0, 1]]},
         {'component': [7, 7, 9], 'condensation': [[7, 9]]},
         {'component': [7, 7, 9], 'condensation': []}),
        ({'task': 'topo', 'n': 3, 'edges': [[0, 2], [1, 2]]},
         {'acyclic': True, 'order': [0, 1, 2]},
         {'acyclic': True, 'order': [1, 0, 2]},
         {'acyclic': True, 'order': [2, 0, 1]}),
        ({'task': 'dag', 'n': 2, 'source': 0, 'edges': [[0, 1, 2], [0, 1, 2]]},
         {'acyclic': True, 'distance': [0, 2], 'parent_edge': [-1, 0]},
         {'acyclic': True, 'distance': [0, 2], 'parent_edge': [-1, 1]},
         {'acyclic': True, 'distance': [0, 2], 'parent_edge': [-1, -1]}),
    ]
    examples.insert(1, copy.deepcopy(examples[0]))
    examples[1][0]['task'] = 'prim'
    for request, expected, alternative, invalid in examples:
        assert check(request, alternative, expected)
        assert not check(request, invalid, expected)
    return [{'task': x[0]['task'], 'alternative_accepted': True, 'invalid_rejected': True}
            for x in examples]


def scale_cases():
    n, m = 100000, 250000
    pairs = [[i, i + 1] for i in range(n - 1)]
    pairs += [[0, n - 1]] * (m - len(pairs))
    states = [[n - i - 1, i + 2] for i in range(n - 1)] + [[1, n]] * (m - n + 1)
    yield {'task': 'dsu', 'n': n, 'operations': pairs}, {'states': states}
    weighted = [[a, b, 1] for a, b in pairs]
    expected_forest = {'cost': n - 1, 'components': 1}
    for task in ('kruskal', 'prim'):
        yield {'task': task, 'n': n, 'edges': weighted}, expected_forest
    directed = [[i, i + 1] for i in range(n - 1)]
    directed += [[0, 1]] * (m - len(directed))
    yield {'task': 'scc', 'n': n, 'edges': directed}, {'component': list(range(n))}
    cycle = directed[:-1] + [[n - 1, 0]]
    yield {'task': 'scc', 'n': n, 'edges': cycle}, {'component': [0] * n}
    yield {'task': 'topo', 'n': n, 'edges': directed}, {'acyclic': True}
    yield {'task': 'topo', 'n': n, 'edges': cycle}, {'acyclic': False}
    yield {'task': 'dag', 'n': n, 'source': 0,
           'edges': [[a, b, 1000000000] for a, b in directed]}, {
               'acyclic': True, 'distance': [i * 1000000000 for i in range(n)]}


def write_lines(name, rows):
    (ROOT / 'tests' / name).write_text(
        ''.join(json.dumps(row, separators=(',', ':')) + '\n' for row in rows), encoding='utf-8')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--full', action='store_true')
    args = parser.parse_args()
    cases = small_cases()
    expected = [reference(x) for x in cases]
    actual = run(cases)
    for i, (x, y, e) in enumerate(zip(cases, actual, expected)):
        assert check(x, y, e), (i, x, y, e)
    repeated = run(cases + cases)
    assert repeated == actual + actual
    write_lines('cases.jsonl', cases)
    write_lines('expected.jsonl', expected)
    write_lines('actual.jsonl', actual)
    summary = {'seed': 20261020, 'small_cases': len(cases),
               'mixed_requests_in_one_process': len(repeated),
               'witness_checks': witness_tests(), 'scale': []}
    if args.full:
        for request, wanted in scale_cases():
            answer = run([request])[0]
            assert check(request, answer, wanted), request['task']
            encoded = json.dumps(answer, separators=(',', ':')).encode()
            summary['scale'].append({
                'task': request['task'], 'n': request['n'],
                'rows': len(request.get('edges', request.get('operations', []))),
                'passed': True, 'actual_bytes': len(encoded),
                'actual_sha256': hashlib.sha256(encoded).hexdigest(),
                'acyclic': answer.get('acyclic'), 'cost': answer.get('cost'),
                'components': answer.get('components'),
                'last_distance': answer.get('distance', [None])[-1],
            })
    (ROOT / 'tests/results.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
