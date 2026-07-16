#!/usr/bin/env python3
"""Validate a Phase J stress report against the checked-in profile."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()
    profile = json.loads(args.profile.read_text(encoding="utf-8"))
    report = json.loads(args.report.read_text(encoding="utf-8"))
    expected = profile["scenarios"]
    actual = {row["name"]: row for row in report.get("scenarios", [])}
    issues: list[str] = []
    if report.get("schema_version") != profile.get("schema_version"):
        issues.append("schema version differs from profile")
    if set(actual) != set(expected):
        issues.append("scenario inventory differs from profile")
    for name, requirement in expected.items():
        row = actual.get(name)
        if row is None:
            continue
        if row.get("status") != "complete":
            issues.append(f"{name}: scenario did not complete")
        if row.get("operation_count", 0) < requirement["min_operation_count"]:
            issues.append(f"{name}: operation count below profile")
    for issue in issues:
        print(issue)
    return 1 if issues else 0


if __name__ == "__main__":
    raise SystemExit(main())
