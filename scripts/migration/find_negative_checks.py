#!/usr/bin/env python3
"""List (and with --write, fix) negative registration checks (one-time step).

An xmake structure test often checked that a target is registered in a
build module and *not* in xmake.lua: `!contains(module, "target(\"x\")") ||
contains(xmake, "target(\"x\")")`. After rewrite_structure_tests.py both
variables read the suite manifest that registers x, so the negative half
must read the root mcpp.toml (which lists members, never test programs).
This rewrites `contains(<var>, "<registration>")` without `!`, where <var>
reads a tests/*/mcpp.toml, to `contains(<read>("mcpp.toml"), ...)`.
"""
import re
import subprocess
import sys
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
files = subprocess.check_output(["git", "diff", "--name-only", "--", "tests"], cwd=ROOT, text=True).split()
write = "--write" in sys.argv
for name in files:
    if not name.endswith(".cpp"):
        continue
    path = ROOT / name
    source = path.read_text(encoding="utf-8")
    variables = {}
    for match in re.finditer(r'const std::string (\w+) =\s*((?:(\w+)\("[^"]+"\)(?:\s*\+\s*)?)+);', source):
        if "/mcpp.toml" in match.group(2) or "build.mcpp" in match.group(2):
            variables[match.group(1)] = match.group(3)
    changed = source
    for var, fn in variables.items():
        pattern = re.compile(r'(?<![!\w])contains\(\s*' + var + r'\s*,(\s*"\\"[A-Za-z0-9_]+\.(?:cpp|mm)\\"")')
        for match in pattern.finditer(source):
            line = source.count("\n", 0, match.start()) + 1
            print(f"{name}:{line}: {match.group(0).strip()}")
        changed = pattern.sub(lambda m: f'contains({fn}("mcpp.toml"),{m.group(1)}', changed)
    if write and changed != source:
        path.write_text(changed, encoding="utf-8", newline="\n")
