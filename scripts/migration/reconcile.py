#!/usr/bin/env python3
"""Map every xmake target of docs/migration/xmake-targets.json to its mcpp
counterpart and write docs/migration/mcpp-target-map.json.

  python scripts/migration/reconcile.py [--listed windows=<file>] [--listed linux=<file>]

A <file> holds `mcpp test --list` output of the workspace (one test name per
line, members in order); with it the script also checks that the number of
mcpp test programs per platform equals the xmake test entries. Exit status 1
when a target has no counterpart or a count differs.
"""
import argparse
import json
import pathlib
import re
import sys
import tomllib

ROOT = pathlib.Path(__file__).resolve().parents[2]
HELPERS = {
    "tests/ui/native_text_shaping_reference.cpp",
    "tests/platform/wayland_test_compositor.cpp",
    "tests/platform/wayland_fake_portal.cpp",
    "tests/platform/wayland_private_session_bus.cpp",
    "tests/platform/wayland_test_background_effect.cpp",
}
# xmake targets with several test cases, split into one mcpp program each.
SPLIT = {
    "hello_window": "examples/hello_window/tests/<case>.cpp, one program per xmake test case",
    "phase_g_test_runner_test": "tests/ui: test_runner_test (case default) and "
                                "test_runner_invalid_environment_test (case invalid_environment)",
}


def members():
    manifest = tomllib.loads((ROOT / "mcpp.toml").read_text(encoding="utf-8"))
    result = {}
    for member in manifest["workspace"]["members"]:
        data = tomllib.loads((ROOT / member / "mcpp.toml").read_text(encoding="utf-8"))
        targets = dict(data.get("targets", {}))
        result[member] = {"targets": targets, "text": (ROOT / member / "mcpp.toml").read_text(encoding="utf-8")}
    return result


def counterpart(name, info, workspace):
    for member, data in workspace.items():
        if name in data["targets"]:
            kind = data["targets"][name].get("kind", "bin")
            return {"member": member, "mcpp": f"[targets.{name}]", "kind": kind}
    mains = [f for f in info["files"] if f.startswith("tests/") and f not in HELPERS]
    if len(mains) == 1:
        suite = mains[0].split("/")[1]
        member = f"tests/{suite}"
        base = mains[0].split("/")[-1]
        if f'"{base}"' in workspace.get(member, {}).get("text", "") or \
                re.search(r'"[a-z]+/[a-z]+_\*\.mm"', workspace.get(member, {}).get("text", "")) and base.endswith(".mm"):
            return {"member": member, "mcpp": f"test program {base[:-len(pathlib.Path(base).suffix)]}",
                    "kind": "test"}
    return None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--listed", action="append", default=[])
    args = parser.parse_args()
    inventory = json.loads((ROOT / "docs/migration/xmake-targets.json").read_text(encoding="utf-8"))
    workspace = members()
    rows, missing = [], []
    for target in inventory["targets"]:
        name = target["name"]
        row = {"xmake": name, "platforms": sorted(target["platforms"])}
        info = next(iter(target["platforms"].values()))
        if name in SPLIT:
            row["mcpp"] = SPLIT[name]
        else:
            found = counterpart(name, info, workspace)
            if found is None:
                missing.append(name)
                row["mcpp"] = None
            else:
                row.update(found)
        rows.append(row)
    counts = {p: c["test_entries"] for p, c in inventory["counts"].items()}
    result = {"generator": "scripts/migration/reconcile.py", "xmake_test_entries": counts,
              "unmapped": missing, "targets": rows}
    status = 0
    for entry in args.listed:
        platform, path = entry.split("=", 1)
        names = [l.strip() for l in pathlib.Path(path).read_text(encoding="utf-8").splitlines()
                 if re.fullmatch(r"[A-Za-z0-9_/]+", l.strip())]
        result.setdefault("mcpp_test_programs", {})[platform] = len(names)
        if len(names) != counts[platform]:
            status = 1
        print(f"{platform}: xmake {counts[platform]} test entries, mcpp {len(names)} test programs")
    lines = [json.dumps({k: v for k, v in result.items() if k != "targets"})[:-1] + ', "targets": [']
    lines += [json.dumps(r, separators=(",", ":")) + "," for r in rows]
    lines[-1] = lines[-1][:-1]
    lines.append("]}")
    out = ROOT / "docs/migration/mcpp-target-map.json"
    out.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    print(f"{len(rows)} xmake targets, {len(missing)} unmapped: {missing}")
    return 1 if missing else status


if __name__ == "__main__":
    sys.exit(main())
