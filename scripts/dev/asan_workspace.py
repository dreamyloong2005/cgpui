"""Writes the AddressSanitizer workspace (docs/getting-started.md, ASan).

usage: asan_workspace.py <windows|linux|macos> <output directory> [--symbols]
       asan_workspace.py members <repository root>
The second form prints `<member>|<bin dir>|<obj dir>` for each member with
a [test] table (test.sh --asan --per-member-cleanup).

The workspace lists every member of the repository's mcpp.toml by a path
relative to <output> and adds the sanitizer flags to [workspace.build].
A [profile] would be the natural place, but a profile's cxxflags reach only
the units of the member a command selects, not its dependencies (measured
with mcpp 2026.11.18.1), and ASan must instrument the whole graph: the MSVC
STL's container annotations are checked at link time to agree in every
object (LNK2038 'annotate_vector'). [workspace.build] reaches every unit.
"""

from __future__ import annotations

import os
import re
import sys
import tomllib
from pathlib import Path

FLAGS = {
    # /MD: the C++ runtime of the SDK packages, which this run stands in for
    # the debug CRT of. No PDB by default: a /DEBUG:FULL PDB is ~45 MB per
    # test program (~25 GB for the suite), so reports name module+offset;
    # --symbols (test.ps1 -AsanSymbols, in build/asan-symbols) adds it for a
    # rerun with function, file and line (the ASan symbolizer does not read
    # a /DEBUG:FASTLINK PDB).
    "windows": {
        "cxx_runtime": "toolchain-coupled",
        "cxxflags": ["/fsanitize=address"],
        "cflags": ["/fsanitize=address"],
        "ldflags": [],
        "symbols": ["/DEBUG:FULL"],
        # Every program begins at scripts/dev/asan_entry_windows.cpp, which
        # ends the process without the DLL detach that hangs under ASan
        # (see that file); test.ps1 -Asan compiles it to asan_entry.obj.
        "defines": ["main=cgpui_asan_main"],
        "entry_obj": "asan_entry.obj",
    },
    # _GLIBCXX_ASSERTIONS: libstdc++ precondition checks (operator[] bounds,
    # front() of an empty container, ...). _GLIBCXX_DEBUG is not used: it
    # changes container layouts, and the system libraries are built without.
    "linux": {
        "cxx_runtime": "self-contained",
        "cxxflags": ["-fsanitize=address,undefined", "-fno-sanitize-recover=undefined",
                     "-fno-omit-frame-pointer", "-D_GLIBCXX_ASSERTIONS"],
        "cflags": ["-fsanitize=address,undefined", "-fno-sanitize-recover=undefined",
                   "-fno-omit-frame-pointer"],
        "ldflags": ["-fsanitize=address,undefined"],
        "symbols": [],
        "defines": [],
    },
    # Apple clang, arm64. _LIBCPP_HARDENING_MODE_EXTENSIVE: libc++'s
    # precondition checks, the counterpart of _GLIBCXX_ASSERTIONS (the debug
    # mode would change the ABI of the containers). Apple's ASan has no
    # LeakSanitizer on arm64 macOS, so leaks are not checked.
    "macos": {
        "cxx_runtime": "self-contained",
        "cxxflags": ["-fsanitize=address,undefined", "-fno-sanitize-recover=undefined",
                     "-fno-omit-frame-pointer",
                     "-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE"],
        "cflags": ["-fsanitize=address,undefined", "-fno-sanitize-recover=undefined",
                   "-fno-omit-frame-pointer"],
        "ldflags": ["-fsanitize=address,undefined"],
        "symbols": [],
        "defines": [],
    },
}


def toml_list(words: list[str]) -> str:
    return "[" + ", ".join(f'"{word}"' for word in words) + "]"


def list_members(repo: Path) -> int:
    text = (repo / "mcpp.toml").read_text("utf-8")
    for member in tomllib.loads(text)["workspace"]["members"]:
        manifest = tomllib.loads((repo / member / "mcpp.toml").read_text("utf-8"))
        if "test" not in manifest:
            continue
        package = manifest["package"]
        obj = f'{package.get("namespace", "")}_{package["name"]}'.lstrip("_")
        print(f'{member}|{package["name"]}|{obj}')
    return 0


def main() -> int:
    if sys.argv[1] == "members":
        return list_members(Path(sys.argv[2]).resolve())
    platform, output = sys.argv[1], Path(sys.argv[2]).resolve()
    repo = Path(__file__).resolve().parents[2]
    flags = dict(FLAGS[platform])
    if "--symbols" in sys.argv[3:]:
        flags["ldflags"] = flags["ldflags"] + flags["symbols"]
    if "entry_obj" in flags:
        flags["ldflags"] = flags["ldflags"] + [(output / flags["entry_obj"]).as_posix()]
    text = (repo / "mcpp.toml").read_text("utf-8")
    members = tomllib.loads(text)["workspace"]["members"]
    relative = Path(os.path.relpath(repo, output)).as_posix()
    listing = "".join(f'    "{relative}/{m}",\n' for m in members)
    text = re.sub(r"members = \[\n.*?\n\]", "members = [\n" + listing + "]",
                  text, count=1, flags=re.S)
    text = text.replace('path = "tools/linux-gcc"',
                        f'path = "{relative}/tools/linux-gcc"')
    text = text.replace('path = "tools/macos-clang"',
                        f'path = "{relative}/tools/macos-clang"')
    text = text.replace('ar = "scripts/dev/macos-ar.py"',
                        f'ar = "{relative}/scripts/dev/macos-ar.py"')
    build = (f'cxx_runtime = "{flags["cxx_runtime"]}"\n'
             f'cxxflags = {toml_list(flags["cxxflags"])}\n'
             f'cflags = {toml_list(flags["cflags"])}\n'
             f'ldflags = {toml_list(flags["ldflags"])}\n'
             # Tests that cannot run under ASan test for it (CGPUI_ASAN).
             f'defines = {toml_list(["CGPUI_ASAN=1"] + flags["defines"])}')
    text, count = re.subn(r'cxx_runtime = "[a-z-]+"', lambda _: build, text)
    if count != 1:
        raise SystemExit("mcpp.toml: expected one [workspace.build] cxx_runtime")
    header = ("# Generated by scripts/dev/asan_workspace.py from the "
              "repository's mcpp.toml; do not edit.\n")
    output.mkdir(parents=True, exist_ok=True)
    target = output / "mcpp.toml"
    if not target.exists() or target.read_text("utf-8") != header + text:
        target.write_text(header + text, "utf-8", newline="\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
