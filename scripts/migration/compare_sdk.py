#!/usr/bin/env python3
"""Compare an mcpp-built SDK manifest with the xmake-built one.

  python scripts/migration/compare_sdk.py <xmake manifest.json> <mcpp manifest.json>

Prints the link order, system libraries, CRT, the files only one package has
and the size of every library in both.
"""
import json
import sys


def main():
    old = json.load(open(sys.argv[1], encoding="utf-8"))
    new = json.load(open(sys.argv[2], encoding="utf-8"))
    print(f"package: {old['package']} -> {new['package']}")
    for key in ("link_names", "system_libraries"):
        print(f"{key}:\n  xmake {old[key]}\n  mcpp  {new[key]}")
    print(f"msvc_runtime: {old.get('msvc_runtime')} -> {new.get('msvc_runtime')}")
    print(f"compiler: {old['compiler'].get('version')} -> {new['compiler'].get('version')}")
    old_files = {f["path"]: f["size"] for f in old["files"]}
    new_files = {f["path"]: f["size"] for f in new["files"]}
    print(f"files: xmake {len(old_files)}, mcpp {len(new_files)}")
    for path in sorted(set(old_files) - set(new_files)):
        print(f"  only xmake: {path} ({old_files[path]} bytes)")
    for path in sorted(set(new_files) - set(old_files)):
        print(f"  only mcpp:  {path} ({new_files[path]} bytes)")
    print("libraries and programs (bytes, xmake -> mcpp):")
    for path in sorted(set(old_files) & set(new_files)):
        if path.startswith(("lib/", "bin/")) and not path.startswith("lib/cmake"):
            print(f"  {path}: {old_files[path]} -> {new_files[path]}"
                  f" ({new_files[path] / max(old_files[path], 1):.2f}x)")
    changed = [p for p in set(old_files) & set(new_files)
               if p.startswith(("include/", "src/")) and old_files[p] != new_files[p]]
    print(f"headers with a different size: {len(changed)}")


if __name__ == "__main__":
    main()
