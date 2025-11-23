# WORKFLOW-004: Automatic Performance Regression Detection

## Metadata
- **ID**: WORKFLOW-004
- **Priority**: MEDIUM
- **Estimated Time**: 1.5 days
- **Dependencies**: REACT-003, REACT-006
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [WORKFLOW_IMPROVEMENTS.md](../../.github/workflows/WORKFLOW_IMPROVEMENTS.md)

---

## Overview

### What are we trying to change?

Build a system that automatically runs performance benchmarks on each PR and blocks the PR if performance regression occurs compared to the baseline.

**Current State**:
- Manual benchmark execution
- No automatic performance regression detection

**Goals**:
- Automatic benchmark execution per PR
- Comparison against baseline
- Fail PR when threshold exceeded
- Performance trend visualization

---

## Changes

### How are we going to implement this?

#### 1. Benchmark Result Comparison Workflow

**`.github/workflows/benchmark-comparison.yaml`**:
```yaml
name: Performance Benchmark Comparison

on:
  pull_request:
    branches: [main]
    paths:
      - 'internal/**'
      - 'core/**'
      - 'benchmarks/**'

jobs:
  benchmark:
    runs-on: ubuntu-latest
    permissions:
      pull-requests: write

    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0

      - name: Setup
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake ninja-build

      - name: Setup vcpkg
        uses: lukka/run-vcpkg@v11

      - name: Build PR Version
        run: |
          cmake -B build-pr -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DBUILD_BENCHMARKS=ON
          cmake --build build-pr

      - name: Run PR Benchmarks
        run: |
          ./build-pr/benchmarks/container_system_benchmarks \
            --benchmark_format=json \
            --benchmark_out=pr_results.json

      - name: Checkout Base Branch
        run: |
          git checkout ${{ github.base_ref }}

      - name: Build Base Version
        run: |
          cmake -B build-base -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DBUILD_BENCHMARKS=ON
          cmake --build build-base

      - name: Run Base Benchmarks
        run: |
          ./build-base/benchmarks/container_system_benchmarks \
            --benchmark_format=json \
            --benchmark_out=base_results.json

      - name: Compare Results
        id: compare
        run: |
          python3 scripts/compare_benchmarks.py \
            --base base_results.json \
            --pr pr_results.json \
            --threshold 0.10 \
            --output comparison.md

      - name: Check for Regression
        run: |
          if [ -f regression_detected ]; then
            echo "Performance regression detected!"
            cat comparison.md
            exit 1
          fi

      - name: Comment on PR
        uses: actions/github-script@v7
        with:
          script: |
            const fs = require('fs');
            const comparison = fs.readFileSync('comparison.md', 'utf8');
            github.rest.issues.createComment({
              issue_number: context.issue.number,
              owner: context.repo.owner,
              repo: context.repo.repo,
              body: comparison
            });
```

#### 2. Benchmark Comparison Script

**`scripts/compare_benchmarks.py`**:
```python
#!/usr/bin/env python3
"""Compare benchmark results and detect regressions."""

import argparse
import json
import sys

def load_benchmarks(filepath):
    """Load benchmark results from JSON file."""
    with open(filepath) as f:
        data = json.load(f)
    return {b['name']: b for b in data['benchmarks']}

def compare(base, pr, threshold):
    """Compare base and PR benchmark results."""
    regressions = []
    improvements = []
    unchanged = []

    for name, base_bench in base.items():
        if name not in pr:
            continue

        pr_bench = pr[name]
        base_time = base_bench['real_time']
        pr_time = pr_bench['real_time']

        change = (pr_time - base_time) / base_time

        result = {
            'name': name,
            'base_time': base_time,
            'pr_time': pr_time,
            'change': change,
            'change_pct': change * 100
        }

        if change > threshold:
            regressions.append(result)
        elif change < -threshold:
            improvements.append(result)
        else:
            unchanged.append(result)

    return regressions, improvements, unchanged

def generate_report(regressions, improvements, unchanged, threshold):
    """Generate markdown report."""
    report = []
    report.append("## Performance Benchmark Report\n")

    if regressions:
        report.append(f"### :x: Regressions (>{threshold*100:.0f}% slower)\n")
        report.append("| Benchmark | Base | PR | Change |")
        report.append("|-----------|------|-----|--------|")
        for r in regressions:
            report.append(f"| {r['name']} | {r['base_time']:.2f}ns | {r['pr_time']:.2f}ns | **+{r['change_pct']:.1f}%** |")
        report.append("")

    if improvements:
        report.append(f"### :white_check_mark: Improvements (>{threshold*100:.0f}% faster)\n")
        report.append("| Benchmark | Base | PR | Change |")
        report.append("|-----------|------|-----|--------|")
        for i in improvements:
            report.append(f"| {i['name']} | {i['base_time']:.2f}ns | {i['pr_time']:.2f}ns | **{i['change_pct']:.1f}%** |")
        report.append("")

    report.append(f"### Summary\n")
    report.append(f"- Regressions: {len(regressions)}")
    report.append(f"- Improvements: {len(improvements)}")
    report.append(f"- Unchanged: {len(unchanged)}")

    return "\n".join(report)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--base', required=True)
    parser.add_argument('--pr', required=True)
    parser.add_argument('--threshold', type=float, default=0.10)
    parser.add_argument('--output', default='comparison.md')
    args = parser.parse_args()

    base = load_benchmarks(args.base)
    pr = load_benchmarks(args.pr)

    regressions, improvements, unchanged = compare(base, pr, args.threshold)
    report = generate_report(regressions, improvements, unchanged, args.threshold)

    with open(args.output, 'w') as f:
        f.write(report)

    if regressions:
        # Create regression detection flag file
        open('regression_detected', 'w').close()
        print(f"REGRESSION DETECTED: {len(regressions)} benchmarks slower than threshold")
        sys.exit(1)

    print("No significant regressions detected")

if __name__ == '__main__':
    main()
```

#### 3. Baseline Update Workflow

**`.github/workflows/update-baseline.yaml`**:
```yaml
name: Update Performance Baseline

on:
  push:
    branches: [main]
    paths:
      - 'internal/**'
      - 'core/**'
  workflow_dispatch:

jobs:
  update-baseline:
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4

      - name: Build and Run Benchmarks
        run: |
          cmake -B build -DBUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
          cmake --build build
          ./build/benchmarks/container_system_benchmarks \
            --benchmark_format=json \
            --benchmark_out=baseline.json

      - name: Update Baseline
        run: |
          python3 scripts/update_baseline.py baseline.json docs/performance/BASELINE.md

      - name: Commit Baseline
        run: |
          git config user.name "github-actions[bot]"
          git config user.email "github-actions[bot]@users.noreply.github.com"
          git add docs/performance/BASELINE.md
          git diff --staged --quiet || git commit -m "Update performance baseline [skip ci]"
          git push
```

---

## Test Plan

### How will we test this?

#### 1. Artificial Regression Test

```cpp
// Intentionally add slow code
void slow_function() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

Create PR and verify regression detection

#### 2. Threshold Validation

```bash
# Test 10% regression threshold
python3 scripts/compare_benchmarks.py \
  --base tests/fixtures/baseline.json \
  --pr tests/fixtures/regression.json \
  --threshold 0.10
```

#### 3. Report Format Verification

```bash
# Generate and verify markdown report
cat comparison.md
```

#### Success Criteria
- [ ] 100% regression detection accuracy
- [ ] PR comments work correctly
- [ ] Automatic baseline updates
- [ ] Visualization dashboard (optional)

---

## Technical Details

### Threshold Settings

| Metric | Warning Threshold | Failure Threshold |
|--------|-------------------|-------------------|
| Serialization | 5% degradation | 10% degradation |
| Deserialization | 5% degradation | 10% degradation |
| Container ops | 5% degradation | 10% degradation |
| Memory usage | 10% increase | 20% increase |

### Noise Reduction

1. **Multiple runs**: Run each benchmark 3 times and use median
2. **Warmup**: Ignore first run
3. **Consistent environment**: Use same GitHub Actions runner

---

## Checklist

- [ ] Write benchmark-comparison.yaml
- [ ] Write compare_benchmarks.py script
- [ ] Write update-baseline.yaml
- [ ] Configure thresholds
- [ ] Test PR comment functionality
- [ ] Artificial regression test
- [ ] Baseline update test

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
