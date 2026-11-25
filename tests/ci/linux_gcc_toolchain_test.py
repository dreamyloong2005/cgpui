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
                    self.assertEqual(tools["sysroot"], "/")
                    self.assertEqual(tools["tools"]["cxx"], "/usr/bin/g++")
                    prefix = (workspace / tools["path"]).resolve()
                    self.assertEqual(prefix, ROOT / "tools/linux-gcc")
                    self.assertFalse(os.path.lexists(prefix / "bin/clang++"))
                    if os.name == "posix":
                        self.assertTrue(os.access(prefix / "bin/g++", os.X_OK))
                        for tool in ("gcc", "ar", "as", "ld"):
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
                f'family = "{tools["family"]}", sysroot = "{tools["sysroot"]}", '
                f'tools = {{ cxx = "{tools["tools"]["cxx"]}" }} }}\n'
                '[build-dependencies]\n'
                f'cgpui.build-support = {{ path = "{(ROOT / "tools/mcpp").as_posix()}" }}\n'
                '[targets.probe]\nkind = "bin"\nmain = "main.cpp"\n')
            gcc_only = '#if !defined(__GNUC__) || defined(__clang__)\n#error Expected GCC\n#endif\n'
            # Match packages/platform/build.mcpp: no standard headers on the
            # importing side, and more than one module in the braced list.
            (project / "build.mcpp").write_text(
                gcc_only + 'import cgpui.build;\n'
                'int main() { cgpui_build::pkg_config({"probe-a", "probe-b"}, false); }\n')
            for suffix in ("a", "b"):
                (project / f"probe-{suffix}.pc").write_text(
                    f'Name: probe-{suffix}\nDescription: GCC probe\nVersion: 1\n'
                    f'Cflags: -DPROBE_{suffix.upper()}=1\n')
            (project / "main.cpp").write_text(
                gcc_only + '#if !PROBE_A || !PROBE_B\n#error Missing pkg-config flags\n#endif\n'
                '#include <array>\n#include <filesystem>\n#include <linux/errno.h>\n'
                'int main() { return std::filesystem::exists(".") ? std::array<int, 1>{0}[0] : 1; }\n')
            subprocess.run([os.environ.get("MCPP", "mcpp"), "run", "probe"],
                           cwd=project, env={**os.environ, "PKG_CONFIG_PATH": str(project)}, check=True)


if __name__ == "__main__":
    unittest.main()
