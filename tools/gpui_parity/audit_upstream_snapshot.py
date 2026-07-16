#!/usr/bin/env python3
"""Audit the strict pinned-upstream snapshot against the CGPUI parity ledger."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path


UPSTREAM_REVISION = "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0"
EXPECTED_REEXPORT_COUNT = 55
EXPECTED_EXAMPLES = {
    "a11y",
    "grid_layout",
    "hello_world",
    "image",
    "input",
    "list_example",
    "mouse_pressure",
    "move_entity_between_windows",
    "on_window_close_quit",
    "opacity",
    "pattern",
    "set_menus",
    "shadow",
    "svg",
    "tab_stop",
    "text",
    "text_wrapper",
    "tree",
    "uniform_list",
    "window_shadow",
}
ACCEPTED_ROW_STATUSES = {"adapted", "non_goal"}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--snapshot", type=Path, required=True)
    parser.add_argument("--ledger", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    snapshot = json.loads(args.snapshot.read_text(encoding="utf-8"))
    ledger = json.loads(args.ledger.read_text(encoding="utf-8"))
    rows = ledger.get("rows", [])
    required_rows = [row for row in rows if row.get("status") == "required"]
    accepted_rows = [
        row for row in rows if row.get("status") in ACCEPTED_ROW_STATUSES
    ]

    issues: list[str] = []
    if snapshot.get("upstream_revision") != UPSTREAM_REVISION:
        issues.append("snapshot revision does not match the pinned revision")
    if ledger.get("upstream_revision") != UPSTREAM_REVISION:
        issues.append("ledger revision does not match the pinned revision")
    if snapshot.get("fetch_mode") != "live":
        issues.append("snapshot was not fetched in strict live mode")
    if snapshot.get("public_reexport_count") != EXPECTED_REEXPORT_COUNT:
        issues.append("public re-export count differs from the pinned snapshot")
    if set(snapshot.get("examples", [])) != EXPECTED_EXAMPLES:
        issues.append("official example inventory differs from the pinned snapshot")
    if required_rows:
        issues.append("required parity rows remain unresolved")
    if len(accepted_rows) != len(rows) or len(rows) != 32:
        issues.append("ledger rows are not fully accepted or the row count changed")

    report = {
        "accepted_rows": len(accepted_rows),
        "example_count": len(snapshot.get("examples", [])),
        "issues": issues,
        "ledger_sha256": sha256(args.ledger),
        "public_reexport_count": snapshot.get("public_reexport_count", 0),
        "required_rows": len(required_rows),
        "snapshot_fetch_mode": snapshot.get("fetch_mode", "unknown"),
        "snapshot_sha256": sha256(args.snapshot),
        "status": "complete" if not issues else "failed",
        "upstream_revision": UPSTREAM_REVISION,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    if issues:
        for issue in issues:
            print(issue)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
