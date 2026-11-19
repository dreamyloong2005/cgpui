#!/usr/bin/env python3
"""Second pass over the rewritten structure tests (one-time step).

rewrite_structure_tests.py mapped literals passed straight to contains();
some tests keep the xmake target names they check in arrays and build
`target("<name>")` around them, and some split one path over two adjacent
literals. This pass, for the files named on the command line:

  * replaces each literal that is an xmake test target name by the file
    name that registers the test under mcpp (`<file>.cpp`, as listed in its
    suite's [test] discover),
  * turns `"target(\"" + x + "\")"` into `"\"" + x + "\""`,
  * joins `"name = \"tests-<suite>\"" "\"<file>.cpp\""` back into one
    registration literal,
  * adds the manifests of the suites those names live in to the read of the
    `manifests` variable.

  python scripts/migration/rewrite_registration_lists.py tests/...cpp ...
"""
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
HELPERS = {
    "tests/ui/native_text_shaping_reference.cpp",
    "tests/platform/wayland_test_compositor.cpp",
    "tests/platform/wayland_fake_portal.cpp",
    "tests/platform/wayland_private_session_bus.cpp",
    "tests/platform/wayland_test_background_effect.cpp",
}


def test_files():
    inventory = json.loads((ROOT / "docs/migration/xmake-targets.json").read_text(encoding="utf-8"))
    result = {}
    for target in inventory["targets"]:
        for info in target["platforms"].values():
            mains = [f for f in info["files"] if f.startswith("tests/") and f not in HELPERS]
            if info["tests"] and len(mains) == 1:
                result[target["name"]] = mains[0]
    return result


def main():
    files = test_files()
    for name in sys.argv[1:]:
        path = ROOT / name
        source = path.read_text(encoding="utf-8")
        suites = set()

        def literal(match):
            text = match.group(1)
            if text in files:
                suites.add(files[text].split("/")[1])
                return '"' + files[text].split("/")[-1] + '"'
            return match.group(0)

        source = re.sub(r'"([A-Za-z0-9_]+)"', literal, source)
        source = re.sub(r'(?:std::string\{)?"target\(\\""\}?(\s*\+\s*)(.+?)(\s*\+\s*)"\\"\)"',
                        r'std::string{"\\""}\1\2\3"\\""', source)
        source = re.sub(r'"name = \\"tests-[a-z-]+\\""\s*\n\s*("\\"[A-Za-z0-9_]+\.cpp\\"")', r"\1", source)
        assign = re.search(r'(const std::string manifests =\s*)((?:read_source\("[^"]+"\)(?:\s*\+\s*)?)+);',
                           source)
        if assign and suites:
            reads = re.findall(r'read_source\("([^"]+)"\)', assign.group(2))
            reads = list(reads)
            for suite in sorted(suites):
                manifest = f"tests/{suite}/mcpp.toml"
                if manifest not in reads:
                    reads.append(manifest)
            text = " +\n      ".join(f'read_source("{r}")' for r in reads)
            source = source[:assign.start(2)] + text + source[assign.end(2):]
        path.write_text(source, encoding="utf-8", newline="\n")
        print(name, sorted(suites))


if __name__ == "__main__":
    main()
