"""Rewrite the target_block()-based run-directory checks (one-time step).

phase_f_cross_platform_test_execution_structure_test and
phase_f_wsl_full_debug_verification_structure_test checked that repository
inspection tests ran from the project root with CGPUI_SOURCE_ROOT set
(xmake rundir/runenvs). Under mcpp the same guarantee is: the test is listed
in its suite's [test] discover, the suite's build.mcpp defines
CGPUI_SOURCE_ROOT (source_root_define(2)), and the test reads that define.
"""
import json
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[2]
inventory = {t["name"]: t for t in json.loads(
    (ROOT / "docs/migration/xmake-targets.json").read_text(encoding="utf-8"))["targets"]}

HELPER = '''bool registered_with_source_root(const char* test) {
  // tests/<suite>/<file>.cpp: listed in the suite's [test] discover, whose
  // build.mcpp defines CGPUI_SOURCE_ROOT, and read through that define.
  const std::string path{test};
  const auto slash = path.rfind('/');
  const std::string suite = path.substr(0, slash);
  const std::string file = path.substr(slash + 1);
  return contains(read_source((suite + "/mcpp.toml").c_str()), ("\\"" + file + "\\"").c_str()) &&
      contains(read_source((suite + "/build.mcpp").c_str()), "source_root_define(2)") &&
      contains(read_source(path.c_str()), "CGPUI_SOURCE_ROOT");
}'''

for name in ("phase_f_cross_platform_test_execution_structure_test",
             "phase_f_wsl_full_debug_verification_structure_test"):
    path = ROOT / "tests/architecture" / (name + ".cpp")
    s = path.read_text(encoding="utf-8")
    s = re.sub(r"std::string target_block\(.*?\n}\n", HELPER + "\n", s, count=1, flags=re.S)
    array = re.search(r"constexpr std::array (\w+)\{(.*?)\};", s, re.S)
    entries = re.findall(r'"(\w+)"', array.group(2))
    files = [next(f for f in inventory[e]["platforms"]["windows"]["files"]) for e in entries]
    body = "".join(f'\n      "{f}",' for f in files)
    s = s[:array.start(2)] + body + "\n  " + s[array.end(2):]
    var = array.group(1)
    s = re.sub(r"    const std::string block = target_block\(manifests, " + var + r"\[index\]\);\n"
               r"    if \(!contains\(block, \"rundir = os.projectdir\(\)\"\) \|\|\n"
               r"        !contains\(block, \"CGPUI_SOURCE_ROOT = os.projectdir\(\)\"\)\) \{",
               f"    if (!registered_with_source_root({var}[index])) {{", s)
    assert "target_block" not in s, name
    path.write_text(s, encoding="utf-8", newline="\n")
    print(name, len(files))
