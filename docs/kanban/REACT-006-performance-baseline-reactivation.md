# REACT-006: Performance Baseline Reactivation

## Metadata
- **ID**: REACT-006
- **Priority**: MEDIUM
- **Estimated Duration**: 0.5 days
- **Dependencies**: REACT-002, REACT-003
- **Status**: TODO
- **Assignee**: TBD
- **Related Documents**: [REACTIVATION_PLAN.md](../advanced/REACTIVATION_PLAN.md)

---

## Overview

### What are we changing?

Update the CI Performance Baseline that depended on integration tests to match the variant-based system to restore automatic performance regression detection.

**Current State**:
- Commented out at `.github/workflows/integration-tests.yml:224-276`
- Test executable: `performance_serialization_performance_test`

---

## Changes

### How are we changing it?

#### 1. Baseline Update (0.25 days)

**New Baseline Values** (based on variant_value_v2):

| Metric | Legacy Baseline | New Baseline | Notes |
|--------|-----------------|--------------|------|
| Serialization | 1.8M ops/s | 5.4M ops/s | 3x improvement |
| Deserialization | 2.1M ops/s | 6.8M ops/s | 3.2x improvement |
| Memory per value | 88 bytes | 56 bytes | 36% reduction |
| Type check | 15 ns | 2 ns | 7.5x improvement |

**Threshold Settings**:
```yaml
performance_thresholds:
  serialization_ops_per_sec:
    min: 4_000_000  # Minimum 4M ops/s
    warning: 5_000_000  # Warning if below 5M
  memory_per_value_bytes:
    max: 70  # Maximum 70 bytes
    warning: 60  # Warning if 60 or above
```

#### 2. CI Workflow Update (0.25 days)

**`.github/workflows/integration-tests.yml` modifications**:
```yaml
# Uncomment and update
performance-baseline:
  needs: [build-and-test]
  runs-on: ubuntu-latest
  steps:
    - name: Run Performance Tests
      run: |
        ./build/tests/performance_serialization_performance_test \
          --benchmark_format=json \
          --benchmark_out=results.json

    - name: Validate Baseline
      run: |
        python scripts/validate_performance.py \
          --results results.json \
          --baseline docs/performance/BASELINE.md \
          --threshold 0.95

    - name: Upload Results
      uses: actions/upload-artifact@v4
      with:
        name: performance-results
        path: results.json
```

---

## Test Plan

### How will we test it?

#### 1. Local Performance Testing
```bash
# Release build for accurate performance measurement
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARKS=ON
cmake --build build

# Run performance tests
./build/tests/performance_serialization_performance_test
```

#### 2. Baseline Comparison
```bash
# Results comparison script
python scripts/validate_performance.py \
  --results results.json \
  --baseline docs/performance/BASELINE.md
```

#### 3. CI Verification
- Automatic performance test execution on PR creation
- PR fails if performance regresses from baseline

#### Success Criteria
- [ ] New baseline values documented
- [ ] CI performance tests enabled
- [ ] Automatic regression detection working

---

## Technical Details

### Performance Validation Script

```python
# scripts/validate_performance.py
import json
import sys

def validate_performance(results_file, baseline_file, threshold):
    with open(results_file) as f:
        results = json.load(f)

    # Parse baseline
    baseline = parse_baseline(baseline_file)

    # Validate each metric
    failures = []
    for benchmark in results['benchmarks']:
        name = benchmark['name']
        value = benchmark['real_time']

        if name in baseline:
            expected = baseline[name]
            if value > expected * (1 / threshold):
                failures.append(f"{name}: {value} > {expected * (1/threshold)}")

    if failures:
        print("Performance regression detected:")
        for f in failures:
            print(f"  - {f}")
        sys.exit(1)

    print("All performance metrics within threshold")
    sys.exit(0)
```

### Baseline Documentation Format

**`docs/performance/BASELINE.md` format**:
```markdown
# Performance Baseline

## System Configuration
- CPU: GitHub Actions runner (2 cores)
- Memory: 7GB
- Compiler: GCC 13 / Clang 17

## Metrics

| Benchmark | Value | Unit | Date |
|-----------|-------|------|------|
| BM_Serialization_Int | 185 | ns | 2025-11-23 |
| BM_Deserialization_Int | 147 | ns | 2025-11-23 |
| BM_TypeCheck | 2.1 | ns | 2025-11-23 |
```

---

## Risks and Mitigations

| Risk | Probability | Impact | Mitigation |
|--------|------|------|-----------|
| CI environment variability | High | Low | Generous threshold |
| False positive | Medium | Low | Warning only, not failure |
| Baseline drift | Low | Medium | Regular baseline refresh |

---

## Checklist

- [ ] Measure new baseline values
- [ ] Update BASELINE.md document
- [ ] Write validate_performance.py script
- [ ] Uncomment integration-tests.yml
- [ ] Configure thresholds
- [ ] Pass local tests
- [ ] Pass CI tests

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-23
