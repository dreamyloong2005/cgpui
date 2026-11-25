"""Check mcpp's macOS toolchain in the source and generated workspaces.

Apple clang leaves C++20 module syntax off; tools/macos-clang/bin/clang++
turns it on, so the cgpui.build module and the build.mcpp programs that
import it compile. mcpp gives its build-program sub-build only the
toolchain path, so the flag must come from the prefix's driver.
"""

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import tomllib
import unittest

ROOT = Path(__file__).resolve().parents[2]


def macos_tools(workspace):
    manifest = tomllib.loads((workspace / "mcpp.toml").read_text("utf-8"))
    return manifest["workspace"]["toolchain"]["macos"]


class MacOSClangTest(unittest.TestCase):
    def test_generated_workspaces(self):
        with tempfile.TemporaryDirectory() as tmp:
            sdk, asan = (Path(tmp) / name for name in ("SDK workspace", "ASan workspace"))
            subprocess.run([sys.executable, ROOT / "scripts/ci/sdk-package.py", "workspace",
                            "--repo-root", ROOT, "--output", sdk, "--cxx-runtime", "host-coupled"],
                           check=True)
            subprocess.run([sys.executable, ROOT / "scripts/dev/asan_workspace.py", "macos", asan],
                           check=True)
            for workspace in (ROOT, sdk, asan):
                with self.subTest(workspace=workspace):
                    tools = macos_tools(workspace)
                    self.assertEqual(tools["family"], "llvm")
                    prefix = (workspace / tools["path"]).resolve()
                    self.assertEqual(prefix, ROOT / "tools/macos-clang")
                    self.assertEqual((workspace / tools["tools"]["ar"]).resolve(),
                                     ROOT / "scripts/dev/macos-ar.py")
                    driver = (prefix / "bin/clang++").read_text("utf-8")
                    self.assertIn("exec /usr/bin/clang++ -fcxx-modules", driver)
                    if os.name == "posix":
                        self.assertTrue(os.access(prefix / "bin/clang++", os.X_OK))
                        for tool in ("clang", "ar", "as", "ld"):
                            self.assertEqual((prefix / "bin" / tool).readlink(),
                                             Path("/usr/bin") / tool)

    @unittest.skipUnless(sys.platform == "darwin", "requires Apple clang")
    def test_build_program_imports_a_module(self):
        tools = macos_tools(ROOT)
        with tempfile.TemporaryDirectory() as tmp:
            project = Path(tmp)
            (project / "helper").mkdir()
            (project / "helper/mcpp.toml").write_text(
                '[package]\nnamespace = "probe"\nname = "helper"\nversion = "0.1.0"\n'
                'standard = "c++23"\n[targets.probe_helper]\nkind = "lib"\n'
                '[build]\nsources = ["helper.cppm"]\n')
            (project / "helper/helper.cppm").write_text(
                'module;\n#include <cstdio>\nexport module probe.helper;\n'
                'export namespace probe { inline int answer() { return 0; } }\n')
            (project / "mcpp.toml").write_text(
                '[package]\nname = "clang-probe"\nversion = "0.1.0"\nstandard = "c++23"\n'
                f'[toolchain]\nmacos = {{ path = "{(ROOT / tools["path"]).as_posix()}", '
                f'family = "{tools["family"]}", tools = {{ ld = "/usr/bin/ld", '
                f'ar = "{(ROOT / tools["tools"]["ar"]).as_posix()}" }} }}\n'
                '[build-dependencies]\nprobe.helper = { path = "helper" }\n'
                '[targets.probe]\nkind = "bin"\nmain = "main.cpp"\n')
            (project / "build.mcpp").write_text(
                'import probe.helper;\nint main() { return probe::answer(); }\n')
            (project / "main.cpp").write_text("int main() { return 0; }\n")
            subprocess.run([os.environ.get("MCPP", "mcpp"), "run", "probe"],
                           cwd=project, check=True)


if __name__ == "__main__":
    unittest.main()
