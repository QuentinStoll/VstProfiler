#!/usr/bin/env python3
"""
Analyse performance stats from a CSV file and print (or save) a summary.

Usage:
    python PerfAnalyser.py <csv_file> [-o <output_file>]

Arguments:
    csv_file        Path to the performance CSV file.
    -o output_file  Optional path to write the summary to instead of stdout.
"""

import argparse
import sys
import csv
from pathlib import Path
from collections import defaultdict


def load_data(path: str) -> dict[str, list[float]]:
    """Load duration_ms values grouped by category."""
    data: dict[str, list[float]] = defaultdict(list)
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            category = row["category"].strip()
            try:
                data[category].append(float(row["duration_ms"]))
            except (ValueError, KeyError):
                pass
    return data


def stats(values: list[float]) -> dict:
    n = len(values)
    mean = sum(values) / n
    sorted_v = sorted(values)
    mid = n // 2
    median = sorted_v[mid] if n % 2 else (sorted_v[mid - 1] + sorted_v[mid]) / 2
    variance = sum((x - mean) ** 2 for x in values) / n
    std = variance ** 0.5
    p95 = sorted_v[int(0.95 * n)]
    p99 = sorted_v[int(0.99 * n)]
    return {
        "count": n,
        "min": sorted_v[0],
        "max": sorted_v[-1],
        "mean": mean,
        "median": median,
        "std": std,
        "p95": p95,
        "p99": p99,
        "total": sum(values),
    }


def format_ms(v: float) -> str:
    """Format a duration value with appropriate precision."""
    if v < 0.01:
        return f"{v * 1000:.3f} µs"
    return f"{v:.6f} ms"


def build_summary(data: dict[str, list[float]], source: str) -> str:
    lines = []
    lines.append("=" * 60)
    lines.append("PERFORMANCE ANALYSIS SUMMARY")
    lines.append(f"Source : {source}")
    lines.append(f"Total records: {sum(len(v) for v in data.values())}")
    lines.append(f"Categories   : {len(data)}")
    lines.append("=" * 60)

    for category in sorted(data.keys()):
        s = stats(data[category])
        lines.append(f"\n{'-' * 60}")
        lines.append(f"  {category}  (n={s['count']})")
        lines.append(f"{'-' * 60}")
        lines.append(f"  {'Min':<10} {format_ms(s['min'])}")
        lines.append(f"  {'Max':<10} {format_ms(s['max'])}")
        lines.append(f"  {'Mean':<10} {format_ms(s['mean'])}")
        lines.append(f"  {'Median':<10} {format_ms(s['median'])}")
        lines.append(f"  {'Std Dev':<10} {format_ms(s['std'])}")
        lines.append(f"  {'P95':<10} {format_ms(s['p95'])}")
        lines.append(f"  {'P99':<10} {format_ms(s['p99'])}")
        lines.append(f"  {'Total':<10} {format_ms(s['total'])}")

        # Highlight any outliers (> mean + 3*std)
        threshold = s["mean"] + 3 * s["std"]
        outliers = [v for v in data[category] if v > threshold]
        if outliers:
            lines.append(
                f"\n  ⚠  {len(outliers)} outlier(s) detected above "
                f"{format_ms(threshold)} (mean + 3σ)"
            )

    lines.append(f"\n{'=' * 60}")

    # Quick narrative summary
    lines.append("\nKEY OBSERVATIONS")
    lines.append("-" * 60)

    # Slowest category by mean
    slowest = max(data, key=lambda c: stats(data[c])["mean"])
    fastest = min(data, key=lambda c: stats(data[c])["mean"])
    highest_var = max(data, key=lambda c: stats(data[c])["std"])

    lines.append(
        f"• Slowest category (mean): {slowest} "
        f"({format_ms(stats(data[slowest])['mean'])} avg)"
    )
    lines.append(
        f"• Fastest category (mean): {fastest} "
        f"({format_ms(stats(data[fastest])['mean'])} avg)"
    )
    lines.append(
        f"• Most variable category : {highest_var} "
        f"(σ = {format_ms(stats(data[highest_var])['std'])})"
    )

    # processBlock specific note if present
    if "processBlock" in data:
        pb = stats(data["processBlock"])
        lines.append(
            f"• processBlock ran {pb['count']} times; "
            f"P99 latency = {format_ms(pb['p99'])}"
        )

    lines.append("=" * 60)
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="Analyse performance stats from a CSV file."
    )
    parser.add_argument("csv_file", help="Path to the performance CSV file")
    parser.add_argument(
        "-o",
        "--output",
        metavar="FILE",
        help="Write summary to FILE instead of printing to terminal",
    )
    args = parser.parse_args()

    csv_path = Path(args.csv_file)
    if not csv_path.exists():
        print(f"Error: file not found — {csv_path}", file=sys.stderr)
        sys.exit(1)

    data = load_data(csv_path)
    if not data:
        print("Error: no data loaded. Check the CSV format.", file=sys.stderr)
        sys.exit(1)

    summary = build_summary(data, source=csv_path.name)

    if args.output:
        out_path = Path(args.output)
        out_path.write_text(summary + "\n", encoding="utf-8")
        print(f"Summary written to {out_path}")
    else:
        print(summary)


if __name__ == "__main__":
    main()