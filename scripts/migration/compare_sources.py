#!/usr/bin/env python3
"""Compare the library sources mcpp compiles with the xmake inventory.

Reads an mcpp compile database (target/<triple>/<fingerprint>/
compile_commands.json) and docs/migration/xmake-targets.json, and reports,
per library, the files only one of the two builds compiles. Exit status 1
when a library differs.

  python scripts/migration/compare_sources.py <compile_commands.json> windows|linux
"""
import json
import pathlib
import re
import sys

# mcpp object directory (obj/<namespace>_<package>) -> xmake target.
LIBRARIES = {
    "cgpui_core": "cgpui_core",
    "cgpui_platform": "cgpui_platform",
    "cgpui_platform-fallback": "cgpui_platform_fallback",
    "cgpui_platform-win32": "cgpui_platform_win32",
    "cgpui_platform-linux-atspi": "cgpui_platform_linux_atspi",
    "cgpui_platform-linux-x11": "cgpui_platform_linux_x11",
    "cgpui_platform-linux-wayland": "cgpui_platform_linux_wayland",
    "cgpui_renderer": "cgpui_renderer",
    "cgpui_renderer-fallback": "cgpui_renderer_fallback",
    "cgpui_renderer-vulkan": "cgpui_renderer_vulkan",
    "cgpui_ui": "cgpui_ui",
    "cgpui_app": "cgpui_app",
}
PLACEHOLDER = "tools/mcpp/empty_library.cpp"


def object_package(entry):
    words = entry.get("arguments") or entry["command"].split()
    for index, word in enumerate(words):
        if word in ("-o", "/Fo") and index + 1 < len(words):
            output = words[index + 1]
            break
    else:
        return None
    match = re.search(r"[\\/]obj[\\/]([^\\/]+)[\\/]", output)
    return match.group(1) if match else None


def main():
    database, platform = sys.argv[1], sys.argv[2]
    root = pathlib.Path(__file__).resolve().parents[2]
    inventory = json.loads((root / "docs/migration/xmake-targets.json").read_text(encoding="utf-8"))
    xmake = {t["name"]: set(t["platforms"][platform]["files"])
             for t in inventory["targets"] if platform in t["platforms"]}
    mcpp = {}
    for entry in json.loads(pathlib.Path(database).read_text(encoding="utf-8")):
        package = object_package(entry)
        if package not in LIBRARIES:
            continue
        source = pathlib.Path(entry["file"]).resolve()
        try:
            relative = source.relative_to(root).as_posix()
        except ValueError:
            continue
        if relative != PLACEHOLDER:
            mcpp.setdefault(LIBRARIES[package], set()).add(relative)
    differs = False
    for package, target in sorted(LIBRARIES.items()):
        expected = xmake.get(target, set())
        actual = mcpp.get(target, set())
        status = "same" if expected == actual else "DIFFERS"
        differs |= expected != actual
        print(f"{target}: xmake {len(expected)}, mcpp {len(actual)} files: {status}")
        for path in sorted(expected - actual):
            print(f"  only xmake: {path}")
        for path in sorted(actual - expected):
            print(f"  only mcpp:  {path}")
    return 1 if differs else 0


if __name__ == "__main__":
    sys.exit(main())
