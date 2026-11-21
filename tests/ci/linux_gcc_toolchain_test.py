"""Check mcpp's Linux toolchain in the source and generated workspaces."""

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import tomllib
import unittest

ROOT = Path(__file__).resolve().parents[2]


def linux_tools(workspace):
    manifest = tomllib.loads((workspace / "mcpp.toml").read_text("utf-8"))
    return manifest["workspace"]["toolchain"]["linux"]


class LinuxGCCTest(unittest.TestCase):
    def test_generated_workspaces(self):
        with tempfile.TemporaryDirectory() as tmp:
            sdk, asan = (Path(tmp) / name for name in ("SDK workspace", "ASan workspace"))
            subprocess.run([sys.executable, ROOT / "scripts/ci/sdk-package.py", "workspace",
                            "--repo-root", ROOT, "--output", sdk, "--cxx-runtime", "host-coupled"],
                           check=True)
            subprocess.run([sys.executable, ROOT / "scripts/dev/asan_workspace.py", "linux", asan],
                           check=True)
            for workspace in (ROOT, sdk, asan):
                with self.subTest(workspace=workspace):
                    tools = linux_tools(workspace)
                    self.assertEqual(tools["family"], "gcc")
                    prefix = (workspace / tools["path"]).resolve()
                    self.assertEqual(prefix, ROOT / "tools/linux-gcc")
                    self.assertFalse(os.path.lexists(prefix / "bin/clang++"))
                    if os.name == "posix":
                        for tool in ("gcc", "g++", "ar", "as", "ld"):
                            self.assertEqual((prefix / "bin" / tool).readlink(), Path("/usr/bin") / tool)

    @unittest.skipUnless(sys.platform.startswith("linux"), "requires a native Linux toolchain")
    def test_mcpp_build_program_and_target_use_gcc(self):
        tools = linux_tools(ROOT)
        with tempfile.TemporaryDirectory() as tmp:
            project = Path(tmp)
            # Use the same Linux selector as the checkout. mcpp itself must
            # propagate it to the separately built and executed build.mcpp.
            (project / "mcpp.toml").write_text(
                '[package]\nname = "gcc-probe"\nversion = "0.1.0"\nstandard = "c++23"\n'
                f'[toolchain]\nlinux = {{ path = "{(ROOT / tools["path"]).as_posix()}", '
                f'family = "{tools["family"]}" }}\n'
                '[targets.probe]\nkind = "bin"\nmain = "main.cpp"\n')
            for name in ("build.mcpp", "main.cpp"):
                (project / name).write_text(
                    '#if !defined(__GNUC__) || defined(__clang__)\n#error Expected GCC\n#endif\n'
                    '#include <array>\nint main() { return std::array<int, 1>{0}[0]; }\n')
            subprocess.run([os.environ.get("MCPP", "mcpp"), "build", "--profile", "dev"],
                           cwd=project, check=True)


if __name__ == "__main__":
    unittest.main()
