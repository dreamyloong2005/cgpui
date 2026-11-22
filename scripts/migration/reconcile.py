#!/usr/bin/env python3
"""Map every xmake target of docs/migration/xmake-targets.json to its mcpp
counterpart and write docs/migration/mcpp-target-map.json.

  python scripts/migration/reconcile.py [--listed macosx=<file>] [--listed linux=<file>] [--listed windows=<file>]

A <file> holds `mcpp test --list` output of the workspace (one test name per
line, members in order); with it the script also checks that the number of
migrated test programs per platform equals the historical xmake test entries,
and that the named post-migration additions are present exactly once.
"""
import argparse
import json
import pathlib
import re
import sys
import tomllib

ROOT = pathlib.Path(__file__).resolve().parents[2]
# Tests added after the xmake inventory was frozen, with the platforms that
# build them (None: every platform). Do not increase the historical baseline
# or let one of these programs hide a missing migrated test.
POST_MIGRATION = {
    "shader_source_structure_test": None,
    "metal_device_pixel_scale_test": {"macosx"},
    "macos_system_appearance_test": {"macosx"},
    "macos_window_backdrop_test": {"macosx"},
    "metal_transparent_surface_test": {"macosx"},
    "macos_client_title_bar_test": {"macosx"},
    "metal_box_shadow_pixel_test": {"macosx"},
    "core_text_glyph_raster_test": {"macosx"},
    "metal_backdrop_blur_pixel_test": {"macosx"},
    "macos_ime_clauses_test": {"macosx"},
    "macos_context_menu_test": {"macosx"},
    # Migrated for Windows and Linux; macOS runs it since Core Text.
    "native_text_shaping_test": {"macosx"},
}
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
        expected = [name for name, platforms in POST_MIGRATION.items()
                    if platforms is None or platform in platforms]
        migrated_count = sum(name not in expected for name in names)
        if (migrated_count != counts[platform] or
                any(names.count(name) != 1 for name in expected)):
            status = 1
        print(f"{platform}: xmake {counts[platform]} test entries, mcpp {len(names)} "
              f"test programs ({migrated_count} migrated)")
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
