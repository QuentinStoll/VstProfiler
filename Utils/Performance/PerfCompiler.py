#!/usr/bin/env python3
"""
PerfCompiler.py
-----------------
Cleans and parses one or more performance log files formatted as:
  [date time] [_perf] [TRACE] [PERF] {nanoseconds}_{start|end} {category}

Produces a single CSV with one row per completed interval per category:
  source_file, category, iteration, start_ns, end_ns, duration_ns, duration_ms

Usage:
  python PerfCompiler.py file1.log [file2.log ...] [-o output.csv]

  -o  Path for the output CSV file (default: ./perf.csv)
"""

import re
import csv
import argparse
from pathlib import Path
from collections import defaultdict

# Matches PERF TRACE lines, e.g.: [25 Feb 2026 7:09:21pm] [_perf] [TRACE] [PERF] 3757286050_start Initialisation
PERF_LINE_RE = re.compile(r"\[TRACE\]\s+\[PERF\]\s+(\d+)_(start|end)\s+(.+)")
MIN_PERF_LINES = 2


# ── Format validation ─────────────────────────────────────────────────────────
def is_valid_log(path: Path) -> bool:
    """Return True if the file contains at least MIN_PERF_LINES valid PERF lines."""
    count = 0
    try:
        with path.open("r", encoding="utf-8", errors="replace") as fh:
            for line in fh:
                if "[TRACE]" in line and "[PERF]" in line and PERF_LINE_RE.search(line):
                    count += 1
                    if count >= MIN_PERF_LINES:
                        return True
    except OSError as e:
        print(f"  [ERROR] Cannot read '{path}': {e}")
    return False



def parse_log(path: Path) -> list[dict]:
    """Parse a single log file; return a list of interval dicts."""
    open_starts: defaultdict[str, list[int]] = defaultdict(list)
    counters:    defaultdict[str, int]        = defaultdict(int)
    rows: list[dict] = []
    skipped    = 0
    perf_lines = 0

    with path.open("r", encoding="utf-8", errors="replace") as fh:
        for raw_line in fh:
            line = raw_line.strip()

            if "[TRACE]" not in line or "[PERF]" not in line:
                skipped += 1
                continue

            m = PERF_LINE_RE.search(line)
            if not m:
                skipped += 1
                continue

            perf_lines += 1
            ns       = int(m.group(1))
            marker   = m.group(2)
            category = m.group(3).strip()

            if marker == "start":
                open_starts[category].append(ns)

            elif marker == "end":
                if not open_starts[category]:
                    print(f"    [WARN] Orphan 'end' for '{category}' at ns={ns}")
                    continue

                start_ns    = open_starts[category].pop()
                duration_ns = ns - start_ns
                counters[category] += 1

                rows.append({
                    "source_file": path.name,
                    "category":    category,
                    "iteration":   counters[category],
                    "start_ns":    start_ns,
                    "end_ns":      ns,
                    "duration_ns": duration_ns,
                    "duration_ms": round(duration_ns / 1_000_000, 6),
                })

    for cat, stack in open_starts.items():
        if stack:
            print(f"    [WARN] {len(stack)} unclosed 'start' event(s) for '{cat}'")

    print(f"    Non-PERF lines skipped : {skipped}")
    print(f"    PERF lines parsed      : {perf_lines}")
    print(f"    Completed intervals    : {len(rows)}")
    print(f"    Categories             : {sorted(counters.keys())}")

    return rows



def write_csv(rows: list[dict], path: Path) -> None:
    if not rows:
        print("\n[!] No data collected — CSV not written.")
        return

    fieldnames = ["source_file", "category", "iteration",
                  "start_ns", "end_ns", "duration_ns", "duration_ms"]

    with path.open("w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

    print(f"\n✓ CSV written → {path}  ({len(rows)} rows total)")



def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Parse one or more performance log files into a single CSV.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="Example:\n  python parse_perf_log.py run1.log run2.log -o results.csv",
    )
    parser.add_argument(
        "inputs",
        nargs="+",
        metavar="INPUT",
        help="One or more .log files to process",
    )
    parser.add_argument(
        "-o", "--output",
        default="./perf.csv",
        metavar="OUTPUT",
        help="Output CSV file path (default: ./perf.csv)",
    )
    return parser



def main() -> None:
    parser = build_parser()
    args   = parser.parse_args()

    seen: set[Path] = set()
    input_paths: list[Path] = []
    for p in (Path(p).resolve() for p in args.inputs):
        if p in seen:
            print(f"[WARN] Duplicate input ignored: {p}")
        else:
            seen.add(p)
            input_paths.append(p)

    output_path = Path(args.output)

    all_rows: list[dict] = []
    accepted = 0
    skipped  = 0

    print(f"Output  : {output_path}")
    print(f"Files   : {len(input_paths)} unique ({len(args.inputs)} provided)\n")

    for path in input_paths:
        print(f"── {path}")

        if not path.exists():
            print(f"  [SKIP] File not found.\n")
            skipped += 1
            continue

        if not is_valid_log(path):
            print(f"  [SKIP] File does not appear to be a valid performance log "
                  f"(fewer than {MIN_PERF_LINES} PERF lines found).\n")
            skipped += 1
            continue

        rows = parse_log(path)
        all_rows.extend(rows)
        accepted += 1
        print()

    print(f"── Summary ────────────────────────────────────")
    print(f"  Accepted : {accepted} file(s)")
    print(f"  Skipped  : {skipped} file(s)")

    write_csv(all_rows, output_path)


if __name__ == "__main__":
    main()