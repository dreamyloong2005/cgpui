#!/usr/bin/env python3
"""Inventory every xmake target before the mcpp migration.

Runs scripts/migration/dump_xmake_targets.lua once per platform (the
is_plat() conditions in xmake.lua are evaluated for that platform) and merges
the results into docs/migration/xmake-targets.json: one entry per target with
the platforms that define it and, per platform, its kind, files, deps,
packages, system links, defines, include dirs and tests (with run dirs and
run environment variable names).

This needs xmake and the xmake build files, so it only runs on a tree that
still has them (master before the migration, e.g. 31607247).
"""
import argparse
import json
import pathlib
import subprocess
import tempfile

PLATFORMS = ("windows", "linux", "macosx")
FIELDS = ("kind", "files", "headerfiles", "deps", "packages", "syslinks",
          "links", "defines", "cxflags", "cxxflags", "ldflags", "includedirs",
          "rundir", "runenvs", "default", "group", "tests")


def dump(root, platform, out):
    script = root / "scripts" / "migration" / "dump_xmake_targets.lua"
    subprocess.run(["xmake", "l", str(script), platform, str(out)],
                   cwd=root, check=True)
    return json.loads(out.read_text(encoding="utf-8"))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".")
    parser.add_argument("--out", default="docs/migration/xmake-targets.json")
    args = parser.parse_args()
    root = pathlib.Path(args.root).resolve()
    merged = {}
    counts = {}
    with tempfile.TemporaryDirectory() as tmp:
        for platform in PLATFORMS:
            data = dump(root, platform, pathlib.Path(tmp) / f"{platform}.json")
            counts[platform] = {
                "targets": data["count"],
                "test_entries": sum(len(t["tests"]) for t in data["targets"]),
            }
            for target in data["targets"]:
                entry = merged.setdefault(target["name"], {"name": target["name"], "platforms": {}})
                entry["platforms"][platform] = {
                    key: (target[key] if target[key] != {} else []) for key in FIELDS}
    result = {
        "source_commit": subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=root, text=True).strip(),
        "generator": "scripts/migration/build_inventory.py",
        "counts": counts,
        "targets": [merged[name] for name in sorted(merged)],
    }
    out = root / args.out
    out.parent.mkdir(parents=True, exist_ok=True)
    # One target per line keeps the file reviewable and diffable.
    head = {key: result[key] for key in ("source_commit", "generator", "counts")}
    lines = [json.dumps(head)[:-1] + ', "targets": [']
    lines += [json.dumps(t, separators=(",", ":")) + "," for t in result["targets"]]
    lines[-1] = lines[-1][:-1]
    lines.append("]}")
    out.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    print(f"{len(merged)} targets -> {out}")
    for platform, count in counts.items():
        print(f"  {platform}: {count['targets']} targets, {count['test_entries']} test entries")


if __name__ == "__main__":
    main()
