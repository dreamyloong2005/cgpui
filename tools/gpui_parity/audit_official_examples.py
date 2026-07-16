#!/usr/bin/env python3
"""Audit Phase J official-example mappings and executable target ownership."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--snapshot", type=Path, required=True)
    parser.add_argument("--matrix", type=Path, required=True)
    parser.add_argument("--targets", type=Path, required=True)
    parser.add_argument("--xmake", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    snapshot = json.loads(args.snapshot.read_text(encoding="utf-8"))
    matrix = json.loads(args.matrix.read_text(encoding="utf-8"))
    rows = matrix.get("examples", [])
    official = set(snapshot.get("examples", []))
    mapped = {row.get("upstream_example") for row in rows}
    canonical = {
        line.strip()
        for line in args.targets.read_text(encoding="utf-8").splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    }
    xmake = args.xmake.read_text(encoding="utf-8")

    issues: list[str] = []
    if matrix.get("upstream_revision") != snapshot.get("upstream_revision"):
        issues.append("matrix revision differs from the strict upstream snapshot")
    if len(rows) != 20 or official != mapped:
        issues.append("official example names are missing, duplicated, or unexpected")
    if len(mapped) != len(rows):
        issues.append("official example mappings contain duplicate names")

    pixel_evidence = 0
    input_evidence = 0
    allowed_targets = canonical | {"hello_window"}
    for row in rows:
        name = row.get("upstream_example", "<unnamed>")
        platforms = row.get("platforms", {})
        verification = row.get("verification", {})
        equivalents = row.get("equivalent_targets", [])
        if row.get("status") != "complete":
            issues.append(f"{name}: mapping is not complete")
        for platform in ("windows", "wayland", "x11"):
            if platforms.get(platform) != "compiled_and_smoked":
                issues.append(f"{name}: {platform} is not compiled_and_smoked")
        if platforms.get("macos") != "excluded_by_user":
            issues.append(f"{name}: macOS execution boundary is not explicit")
        if not equivalents or any(target not in allowed_targets for target in equivalents):
            issues.append(f"{name}: equivalent target is absent from the canonical inventory")
        for target in equivalents:
            if f'target("{target}")' not in xmake:
                issues.append(f"{name}: {target} has no Xmake target")
        if not verification.get("smoke_tests"):
            issues.append(f"{name}: smoke evidence is empty")
        pixel_evidence += len(verification.get("pixel_tests", []))
        input_evidence += len(verification.get("input_tests", []))
    if pixel_evidence == 0 or input_evidence == 0:
        issues.append("matrix lacks pixel or input evidence")

    report = {
        "input_evidence": input_evidence,
        "issues": issues,
        "macos_execution": "excluded_by_user",
        "mapped_examples": len(mapped),
        "matrix_sha256": sha256(args.matrix),
        "official_examples": len(official),
        "pixel_evidence": pixel_evidence,
        "snapshot_sha256": sha256(args.snapshot),
        "status": "complete" if not issues else "failed",
        "target_count": len(canonical),
        "upstream_revision": snapshot.get("upstream_revision", ""),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    for issue in issues:
        print(issue)
    return 1 if issues else 0


if __name__ == "__main__":
    raise SystemExit(main())
