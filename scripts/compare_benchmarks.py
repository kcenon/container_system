#!/usr/bin/env python3
"""
Compare benchmark results and detect performance regressions.

This script compares benchmark results from a PR against a baseline
and generates a markdown report with the comparison.
"""

import argparse
import json
import sys
from pathlib import Path
from typing import Any


def load_benchmarks(filepath: str) -> dict[str, dict[str, Any]]:
    """Load benchmark results from JSON file."""
    with open(filepath) as f:
        data = json.load(f)

    # Handle Google Benchmark output format
    benchmarks = {}
    for bench in data.get('benchmarks', []):
        name = bench.get('name', '')
        # Skip aggregate entries (mean, median, stddev)
        if '_mean' in name or '_median' in name or '_stddev' in name or '_cv' in name:
            continue
        benchmarks[name] = bench

    return benchmarks


def compare_benchmarks(
    base: dict[str, dict[str, Any]],
    pr: dict[str, dict[str, Any]],
    threshold: float
) -> tuple[list[dict], list[dict], list[dict]]:
    """Compare base and PR benchmark results.

    Args:
        base: Baseline benchmark results
        pr: PR benchmark results
        threshold: Regression threshold (e.g., 0.10 for 10%)

    Returns:
        Tuple of (regressions, improvements, unchanged)
    """
    regressions = []
    improvements = []
    unchanged = []

    for name, base_bench in base.items():
        if name not in pr:
            continue

        pr_bench = pr[name]

        # Use real_time for comparison (nanoseconds)
        base_time = base_bench.get('real_time', 0)
        pr_time = pr_bench.get('real_time', 0)

        if base_time == 0:
            continue

        change = (pr_time - base_time) / base_time

        result = {
            'name': name,
            'base_time': base_time,
            'pr_time': pr_time,
            'change': change,
            'change_pct': change * 100,
            'time_unit': base_bench.get('time_unit', 'ns')
        }

        if change > threshold:
            regressions.append(result)
        elif change < -threshold:
            improvements.append(result)
        else:
            unchanged.append(result)

    return regressions, improvements, unchanged


def format_time(value: float, unit: str) -> str:
    """Format time value with appropriate unit."""
    if unit == 'ns' and value >= 1000000:
        return f"{value / 1000000:.2f}ms"
    elif unit == 'ns' and value >= 1000:
        return f"{value / 1000:.2f}us"
    return f"{value:.2f}{unit}"


def generate_report(
    regressions: list[dict],
    improvements: list[dict],
    unchanged: list[dict],
    threshold: float
) -> str:
    """Generate markdown report."""
    lines = []
    lines.append("## Performance Benchmark Report\n")

    # Summary first
    total = len(regressions) + len(improvements) + len(unchanged)
    lines.append("### Summary\n")
    lines.append(f"- **Total benchmarks**: {total}")
    lines.append(f"- **Regressions**: {len(regressions)} :x:" if regressions else f"- **Regressions**: 0 :white_check_mark:")
    lines.append(f"- **Improvements**: {len(improvements)}")
    lines.append(f"- **Unchanged**: {len(unchanged)} (within {threshold*100:.0f}% threshold)")
    lines.append("")

    # Regressions (if any)
    if regressions:
        lines.append(f"### :x: Regressions (>{threshold*100:.0f}% slower)\n")
        lines.append("| Benchmark | Base | PR | Change |")
        lines.append("|-----------|------|-----|--------|")
        for r in sorted(regressions, key=lambda x: x['change'], reverse=True):
            base_str = format_time(r['base_time'], r['time_unit'])
            pr_str = format_time(r['pr_time'], r['time_unit'])
            lines.append(f"| `{r['name']}` | {base_str} | {pr_str} | **+{r['change_pct']:.1f}%** :warning: |")
        lines.append("")

    # Improvements (if any)
    if improvements:
        lines.append(f"### :rocket: Improvements (>{threshold*100:.0f}% faster)\n")
        lines.append("| Benchmark | Base | PR | Change |")
        lines.append("|-----------|------|-----|--------|")
        for i in sorted(improvements, key=lambda x: x['change']):
            base_str = format_time(i['base_time'], i['time_unit'])
            pr_str = format_time(i['pr_time'], i['time_unit'])
            lines.append(f"| `{i['name']}` | {base_str} | {pr_str} | **{i['change_pct']:.1f}%** :chart_with_downwards_trend: |")
        lines.append("")

    # Result status
    lines.append("---")
    if regressions:
        lines.append(":x: **Status**: Performance regressions detected. Please review the changes above.")
    else:
        lines.append(":white_check_mark: **Status**: No significant performance regressions detected.")

    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description='Compare benchmark results and detect regressions'
    )
    parser.add_argument(
        '--base',
        required=True,
        help='Path to baseline benchmark results JSON'
    )
    parser.add_argument(
        '--pr',
        required=True,
        help='Path to PR benchmark results JSON'
    )
    parser.add_argument(
        '--threshold',
        type=float,
        default=0.10,
        help='Regression threshold (default: 0.10 for 10%%)'
    )
    parser.add_argument(
        '--output',
        default='comparison.md',
        help='Output markdown file path'
    )

    args = parser.parse_args()

    # Validate input files
    if not Path(args.base).exists():
        print(f"Error: Base results file not found: {args.base}")
        sys.exit(1)
    if not Path(args.pr).exists():
        print(f"Error: PR results file not found: {args.pr}")
        sys.exit(1)

    # Load and compare
    base = load_benchmarks(args.base)
    pr = load_benchmarks(args.pr)

    if not base:
        print("Warning: No benchmarks found in base results")
    if not pr:
        print("Warning: No benchmarks found in PR results")

    regressions, improvements, unchanged = compare_benchmarks(
        base, pr, args.threshold
    )

    # Generate report
    report = generate_report(regressions, improvements, unchanged, args.threshold)

    # Write report
    with open(args.output, 'w') as f:
        f.write(report)
    print(f"Report written to {args.output}")

    # Create regression flag file if regressions detected
    if regressions:
        Path('regression_detected').touch()
        print(f"REGRESSION DETECTED: {len(regressions)} benchmark(s) slower than {args.threshold*100:.0f}% threshold")
        for r in regressions:
            print(f"  - {r['name']}: +{r['change_pct']:.1f}%")
        sys.exit(1)

    print(f"No significant regressions detected (threshold: {args.threshold*100:.0f}%)")
    print(f"  - Improvements: {len(improvements)}")
    print(f"  - Unchanged: {len(unchanged)}")


if __name__ == '__main__':
    main()
