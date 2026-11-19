#!/usr/bin/env python3
"""Third pass: `target("<name>")` literals kept in arrays (one-time step).

Maps every remaining "target(\"<name>\")" literal of the named files to the
mcpp registration text (rewrite_structure_tests.map_literal) and adds the
manifests holding them to the read of `manifests`. With --paths, also maps
"examples/api_parity/<dir>/main.cpp" literals to their `main = ...` entry.

  python scripts/migration/rewrite_target_arrays.py [--paths] tests/...cpp ...
"""
import pathlib
import re
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import rewrite_structure_tests as base  # noqa: E402


def main():
    paths = "--paths" in sys.argv
    regs = base.registrations()
    for name in [a for a in sys.argv[1:] if not a.startswith("--")]:
        path = base.ROOT / name
        source = path.read_text(encoding="utf-8")
        manifests = []

        def literal(match):
            text = base.decode(match.group(0))
            if not (text.startswith("target(") or (paths and text.startswith("examples/"))):
                return match.group(0)
            mapped = base.map_literal(text, regs)
            if mapped is None:
                print(f"  unmapped {name}: {match.group(0)}")
                return match.group(0)
            if mapped[0] not in manifests:
                manifests.append(mapped[0])
            return base.encode(mapped[1])

        source = base.LITERAL.sub(literal, source)
        assign = re.search(r'(const std::string manifests =\s*)((?:read_source\("[^"]+"\)(?:\s*\+\s*)?)+);', source)
        if assign and manifests:
            reads = re.findall(r'read_source\("([^"]+)"\)', assign.group(2))
            reads += [m for m in manifests if m not in reads]
            text = " +\n      ".join(f'read_source("{r}")' for r in reads)
            source = source[:assign.start(2)] + text + source[assign.end(2):]
        path.write_text(source, encoding="utf-8", newline="\n")
        print(name, manifests)


if __name__ == "__main__":
    main()
