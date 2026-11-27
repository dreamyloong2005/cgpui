#!/usr/bin/env python3
"""Check real baseline runner binaries, including zero/partial-work mutations."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile


METRICS = (
    "startup", "first_frame", "resize", "text_layout", "list_scrolling",
    "image_loading", "async_wakeup", "frame_pacing",
)
SENTINEL = b"existing report must survive a rejected workload\n"


class VerificationError(Exception):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise VerificationError(message)


def run(runner: Path, output: Path, args: argparse.Namespace) -> subprocess.CompletedProcess[str]:
    try:
        return subprocess.run(
            [str(runner), "--platform", args.platform, "--iterations",
             str(args.iterations), "--output", str(output)],
            capture_output=True, text=True, encoding="utf-8", errors="replace",
            timeout=args.timeout, check=False,
        )
    except (OSError, subprocess.TimeoutExpired) as error:
        raise VerificationError(f"{runner}: runner did not execute: {error}") from error


def result_detail(result: subprocess.CompletedProcess[str]) -> str:
    return (f"exit={result.returncode}; stdout={result.stdout[-1200:]!r}; "
            f"stderr={result.stderr[-1200:]!r}")


def check_normal(runner: Path, directory: Path, args: argparse.Namespace) -> None:
    output = directory / "normal.json"
    result = run(runner, output, args)
    require(result.returncode == 0, f"normal runner failed: {result_detail(result)}")
    require(output.is_file(), "normal runner did not publish its report")
    try:
        report = json.loads(output.read_text(encoding="utf-8"))
    except (OSError, ValueError) as error:
        raise VerificationError(f"normal report is unreadable: {error}") from error
    require(isinstance(report, dict), "normal report is not an object")
    require(type(report.get("schema_version")) is int and report["schema_version"] == 1,
            "normal report schema differs")
    require(report.get("platform") == args.platform, "normal report platform differs")
    require(type(report.get("iterations")) is int and report["iterations"] == args.iterations,
            "normal report iterations differ")
    rows = report.get("metrics")
    require(isinstance(rows, list) and len(rows) == len(METRICS),
            "normal report must contain exactly eight workloads")
    counts = (args.iterations,) * 6 + (64, 10)
    for name, expected, row in zip(METRICS, counts, rows):
        require(isinstance(row, dict) and row.get("name") == name,
                f"normal report has missing, duplicate, or reordered workload {name}")
        require(row.get("unit") == "ns_per_operation", f"{name}: unexpected metric unit")
        count, total, mean = (row.get(key) for key in ("operation_count", "total_ns", "mean_ns"))
        require(type(count) is int and count == expected,
                f"{name}: expected {expected} completed operations, got {count!r}")
        require(type(total) is int and total >= 0 and type(mean) is int and mean >= 0,
                f"{name}: invalid timing values")
        require(mean == total // count, f"{name}: mean does not use the completed count")
    print("PASS normal: all eight workloads completed with observed operation counts")


def check_rejected(runner: Path, completed: int, directory: Path,
                   args: argparse.Namespace) -> None:
    for existing in (False, True):
        label = f"async-{completed}-of-64-{'existing' if existing else 'fresh'}"
        output = directory / f"{label}.json"
        if existing:
            output.write_bytes(SENTINEL)
        result = run(runner, output, args)
        require(result.returncode == 4,
                f"{label}: expected workload rejection, not build/crash failure: {result_detail(result)}")
        require("async_wakeup" in result.stderr and f"({completed}/64," in result.stderr,
                f"{label}: missing actual completion diagnostic: {result_detail(result)}")
        require('"schema_version"' not in result.stdout,
                f"{label}: rejected workload published a report on stdout")
        if existing:
            require(output.is_file() and output.read_bytes() == SENTINEL,
                    f"{label}: rejected workload replaced the existing output")
        else:
            require(not output.exists(), f"{label}: rejected workload created an output")
        print(f"PASS {label}: exit 4, diagnosed {completed}/64, no report publication")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--normal-runner", type=Path, required=True)
    parser.add_argument("--zero-runner", type=Path, required=True)
    parser.add_argument("--partial-runner", type=Path, required=True)
    parser.add_argument("--iterations", type=int, default=4)
    parser.add_argument("--platform", default="portable",
                        choices=("portable", "windows", "wayland", "x11", "macos-arm64"))
    parser.add_argument("--timeout", type=float, default=120,
                        help="maximum seconds for each actual runner invocation")
    args = parser.parse_args()
    if not 1 <= args.iterations <= 10000 or not 0 < args.timeout <= 3600:
        parser.error("iterations must be 1..10000 and timeout must be 0..3600 seconds")
    runners = [path.resolve() for path in
               (args.normal_runner, args.zero_runner, args.partial_runner)]
    try:
        require(len(set(runners)) == 3, "provide three distinct built runner binaries")
        for runner in runners:
            require(runner.is_file(), f"runner does not exist: {runner}")
            print(f"RUNNER {runner}: sha256={hashlib.sha256(runner.read_bytes()).hexdigest()}")
        with tempfile.TemporaryDirectory(prefix="cgpui-performance-sensitivity-") as temporary:
            directory = Path(temporary)
            check_normal(runners[0], directory, args)
            check_rejected(runners[1], 0, directory, args)
            check_rejected(runners[2], 32, directory, args)
    except (VerificationError, OSError) as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    print("PASS performance workload sensitivity: five actual CLI executions")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
