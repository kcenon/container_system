# Automated Performance Tracking System

> **System Status**: ✅ Active
> **Last Updated**: 2025-11-07

## Overview

The container system includes an automated performance tracking system that continuously monitors memory pool performance across multiple platforms. Results are automatically collected, analyzed, and published to documentation.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│           GitHub Actions Workflow                       │
│  (.github/workflows/memory-pool-benchmarks.yml)        │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  1. Trigger Events                                      │
│     - Push to main                                      │
│     - Pull request (memory pool changes)                │
│     - Manual dispatch                                   │
│                                                         │
│  2. Benchmark Execution (Multi-platform)                │
│     ├─ Ubuntu 22.04 (x86_64)                           │
│     └─ macOS latest (ARM64)                            │
│                                                         │
│  3. Results Collection                                  │
│     - JSON format (Google Benchmark)                    │
│     - 5 repetitions for statistical confidence          │
│     - Aggregate metrics (mean, median, stddev)          │
│                                                         │
│  4. Automated Analysis                                  │
│     - Parse benchmark JSON                              │
│     - Calculate performance metrics                     │
│     - Compare pool vs standard allocator                │
│     - Generate markdown reports                         │
│                                                         │
│  5. Documentation Update                                │
│     - Update MEMORY_POOL_PERFORMANCE.md                 │
│     - Auto-commit with [skip ci] tag                    │
│     - Add PR comments (for pull requests)               │
│                                                         │
│  6. Artifact Storage                                    │
│     - Store JSON results (90-day retention)             │
│     - Enable historical comparison                      │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

## Workflow Details

### File: `.github/workflows/memory-pool-benchmarks.yml`

#### Trigger Conditions

```yaml
on:
  push:
    branches: [ main, claude/* ]
    paths:
      - 'internal/memory_pool.h'
      - 'tests/memory_pool_benchmark.cpp'
  pull_request:
    branches: [ main ]
  workflow_dispatch:
    inputs:
      update_docs:
        description: 'Update performance documentation'
        default: 'true'
```

#### Jobs

1. **benchmark-memory-pool**
   - Runs on Ubuntu 22.04 and macOS latest
   - Builds `memory_pool_benchmark` executable
   - Executes benchmarks with 5 repetitions
   - Outputs JSON and console results
   - Uploads artifacts for 90 days

2. **update-documentation**
   - Runs after benchmarks complete
   - Only on `main` branch or manual trigger
   - Downloads all benchmark results
   - Parses JSON and extracts metrics
   - Generates markdown tables
   - Commits changes automatically

3. **summary**
   - Creates GitHub Actions summary
   - Links to artifacts and documentation

## Automated Documentation Updates

### Target File: `docs/MEMORY_POOL_PERFORMANCE.md`

The workflow automatically updates this file with:
- Latest benchmark results
- Performance comparison tables (pool vs standard allocator)
- Speedup factors
- Platform-specific results
- Key performance insights
- Recommendations

### Update Process

```python
# Simplified workflow logic

1. Parse JSON benchmark results:
   - Extract benchmark names and times
   - Filter for key benchmarks (64B, 256B, 1KB, etc.)
   - Calculate aggregate statistics

2. Calculate performance metrics:
   - Pool allocation time
   - Standard allocator time
   - Speedup factor = standard_time / pool_time

3. Generate markdown tables:
   - Platform-specific results (Ubuntu, macOS)
   - Comparison with standard allocator
   - Key insights and recommendations

4. Write to documentation:
   - Update docs/MEMORY_POOL_PERFORMANCE.md
   - Include timestamp and metadata

5. Commit and push:
   - git add docs/MEMORY_POOL_PERFORMANCE.md
   - git commit -m "docs: Update memory pool performance results [skip ci]"
   - git push
```

### Example Generated Output

```markdown
## Memory Pool Performance Results

> **Last Updated**: 2025-11-07 14:23:45 UTC
> **Auto-generated by**: GitHub Actions

### Ubuntu 22.04 (x86_64)

| Benchmark | Time (ns) | vs Standard Allocator |
|-----------|-----------|----------------------|
| BM_PoolAllocation_64B | 42.3 | 8.2x faster |
| BM_StandardAllocation_64B | 346.7 | baseline |
| BM_PoolAllocation_256B | 45.1 | 6.5x faster |
| BM_StandardAllocation_256B | 293.2 | baseline |

## Key Performance Insights

- **64B blocks**: Pool is **8.2x faster** than standard allocator (Ubuntu)
- **256B blocks**: Pool is **6.5x faster** than standard allocator (Ubuntu)
```

## Manual Workflow Execution

### Via GitHub UI

1. Navigate to: **Actions** → **Memory Pool Performance Benchmarks**
2. Click **Run workflow**
3. Select branch (default: main)
4. Choose whether to update docs (default: true)
5. Click **Run workflow**

### Via GitHub CLI

```bash
# Trigger workflow manually
gh workflow run memory-pool-benchmarks.yml

# Trigger and update docs
gh workflow run memory-pool-benchmarks.yml \
  --field update_docs=true

# View workflow runs
gh run list --workflow=memory-pool-benchmarks.yml

# View specific run
gh run view <run-id>
```

## Accessing Benchmark Results

### Latest Results

📊 **Live Data**: [docs/MEMORY_POOL_PERFORMANCE.md](MEMORY_POOL_PERFORMANCE.md)

### Historical Results

1. Go to **Actions** → **Memory Pool Performance Benchmarks**
2. Select a workflow run
3. Scroll to **Artifacts** section
4. Download:
   - `memory-pool-results-ubuntu-22.04` (JSON)
   - `memory-pool-results-macos-latest` (JSON)

### Artifact Contents

```json
{
  "context": {
    "date": "2025-11-07T14:23:45Z",
    "host_name": "github-runner",
    "executable": "./memory_pool_benchmark",
    "num_cpus": 4
  },
  "benchmarks": [
    {
      "name": "BM_PoolAllocation_64B_mean",
      "iterations": 16420350,
      "real_time": 42.3,
      "cpu_time": 42.3,
      "time_unit": "ns"
    }
  ]
}
```

## Performance Analysis

### Comparing Results

```bash
# Download two result files
# Compare using Google Benchmark compare.py

curl -O https://raw.githubusercontent.com/google/benchmark/main/tools/compare.py

python3 compare.py \
  baseline_results.json \
  current_results.json
```

### Key Metrics to Monitor

1. **Pool vs Standard Allocator Speedup**
   - Target: 10-50x for small blocks (<256B)
   - Acceptable: 5-20x for medium blocks (256B-1KB)
   - Expected: 2-10x for large blocks (>1KB)

2. **Cross-Platform Consistency**
   - Results should be similar across platforms
   - ARM (macOS) may show different characteristics

3. **Regression Detection**
   - Speedup decreasing over time
   - Absolute time increasing significantly
   - Standard deviation increasing (instability)

## Pull Request Integration

When a PR modifies memory pool code:

1. **Automatic Benchmark Execution**
   - Benchmarks run on PR creation/update
   - Results added as PR comment

2. **PR Comment Format**

```markdown
## 🚀 Memory Pool Performance Results

### Ubuntu 22.04 (x86_64)

| Benchmark | Time (ns) | vs Standard Allocator |
|-----------|-----------|----------------------|
| BM_PoolAllocation_64B | 42.3 | 8.2x faster |

**Key Changes**:
- Pool allocation 8.2x faster than standard allocator
- Performance stable across platforms

---
*Auto-generated by GitHub Actions*
```

3. **Review Checklist**
   - ✅ No performance regressions
   - ✅ Speedup factors within expected ranges
   - ✅ Cross-platform consistency maintained

## Maintenance

### Workflow Updates

To modify the automated tracking system:

1. Edit `.github/workflows/memory-pool-benchmarks.yml`
2. Test changes on a feature branch
3. Verify workflow runs successfully
4. Merge to main

### Troubleshooting

**Issue**: Benchmark executable not found

```yaml
# Solution: Check build configuration
- name: Build memory pool benchmark
  run: |
    cmake --build build --target memory_pool_benchmark -j
    # Verify executable exists
    ls -la build/memory_pool_benchmark || ls -la build/bin/memory_pool_benchmark
```

**Issue**: Documentation not updating

```yaml
# Solution: Check permissions
permissions:
  contents: write  # Required for auto-commit
```

**Issue**: Parse errors in results

```python
# Solution: Validate JSON format
try:
    with open(results_file, 'r') as f:
        data = json.load(f)
        print(f"Loaded {len(data.get('benchmarks', []))} benchmarks")
except json.JSONDecodeError as e:
    print(f"JSON parse error: {e}")
```

## Best Practices

### For Developers

1. **Check Performance Before PR**
   ```bash
   # Run local benchmarks
   cd build
   ./memory_pool_benchmark
   ```

2. **Review Automated Results**
   - Check PR comments for performance data
   - Investigate any regressions
   - Update expectations if intentional changes

3. **Manual Trigger for Baseline**
   - After significant optimization
   - When establishing new baseline
   - For documentation refresh

### For Maintainers

1. **Monitor Workflow Health**
   - Check for failed runs
   - Review artifact storage usage
   - Update dependencies periodically

2. **Performance Regression Policy**
   - >10% regression requires investigation
   - >20% regression requires justification
   - >50% regression blocks merge

3. **Documentation Currency**
   - Results auto-update on main branch
   - Manual update available via workflow dispatch
   - Historical data retained for 90 days

## Security Considerations

### Workflow Security

- Uses `GITHUB_TOKEN` with minimal permissions
- Auto-commit limited to documentation files
- `[skip ci]` prevents infinite loops
- No secrets or credentials in benchmarks

### Data Privacy

- Benchmark results are public (in public repos)
- No sensitive system information exposed
- Hardware details limited to CPU count and OS

## Future Enhancements

### Planned Features

1. **Performance Regression Alerts**
   - Automatic issue creation on regression
   - Email notifications to maintainers

2. **Historical Trend Analysis**
   - Long-term performance graphs
   - Statistical anomaly detection

3. **Baseline Management**
   - Automated baseline updates
   - Comparison with historical baselines

4. **Multi-Architecture Support**
   - Windows builds
   - Additional Linux distributions
   - ARM32 platforms

## References

- [Google Benchmark Documentation](https://github.com/google/benchmark)
- [GitHub Actions Workflow Syntax](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)
- [Memory Pool Implementation](../internal/memory_pool.h)
- [Benchmark Tests](../tests/memory_pool_benchmark.cpp)

---

**Document Version**: 1.0
**Last Updated**: 2025-11-07
**Maintainer**: Container System Development Team
**Contact**: kcenon@naver.com
