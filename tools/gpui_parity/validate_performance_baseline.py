#!/usr/bin/env python3
"""Validate a Phase J performance report against the checked-in policy."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--policy", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()
    policy = json.loads(args.policy.read_text(encoding="utf-8"))
    report = json.loads(args.report.read_text(encoding="utf-8"))
    expected = policy["metrics"]
    actual = {row["name"]: row for row in report.get("metrics", [])}
    issues: list[str] = []
    if report.get("schema_version") != policy.get("schema_version"):
        issues.append("schema version differs from policy")
    if set(actual) != set(expected):
        issues.append("metric inventory differs from policy")
    for name, budget in expected.items():
        row = actual.get(name)
        if row is None:
            continue
        if row.get("unit") != policy["metric_unit"]:
            issues.append(f"{name}: unexpected unit")
        if row.get("operation_count", 0) < budget["min_operation_count"]:
            issues.append(f"{name}: operation count below policy")
        if row.get("mean_ns", -1) < 0:
            issues.append(f"{name}: invalid mean")
        elif row["mean_ns"] > budget["max_mean_ns"]:
            issues.append(f"{name}: mean exceeds max_mean_ns")
    for issue in issues:
        print(issue)
    return 1 if issues else 0


if __name__ == "__main__":
    raise SystemExit(main())
