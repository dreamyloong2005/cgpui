"""Experimental DX12 selection must preserve a complete, platform-scoped SDK graph."""
import importlib.util
from pathlib import Path
import sys
import re
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts/ci"))
SPEC = importlib.util.spec_from_file_location("sdk_package", ROOT / "scripts/ci/sdk-package.py")
PACKER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PACKER)


class Dx12PackageGraphTest(unittest.TestCase):
    def test_direct_windows_package_copies_current_framework_link_closure(self):
        script = (ROOT / "scripts/ci/windows-package.ps1").read_text("utf-8")
        table = re.search(r'\$libraries = \[ordered\]@\{(.*?)\n\}', script, re.S)
        self.assertIsNotNone(table)
        copied = dict(re.findall(r'"([^"\n]+)\.lib"\s*=\s*"([^"\n]+)"', table.group(1)))
        for library in PACKER.link_order(ROOT, "windows"):
            if library["origin"] == "cgpui":
                self.assertEqual(copied.get(library["name"]), library["package"])

    def test_windows_selector_and_both_backends_are_in_link_order(self):
        names = [entry["package"] for entry in PACKER.link_order(ROOT, "windows")]
        for name in ("renderer-windows", "renderer-dx12", "renderer-vulkan", "renderer"):
            self.assertEqual(names.count(name), 1, name)
        for backend in ("renderer-dx12", "renderer-vulkan"):
            self.assertLess(names.index("renderer-windows"), names.index(backend))
            self.assertLess(names.index(backend), names.index("renderer"))

    def test_other_platforms_do_not_link_windows_backends(self):
        for platform in ("linux", "macos"):
            with self.subTest(platform=platform):
                names = [entry["package"] for entry in PACKER.link_order(ROOT, platform)]
                self.assertNotIn("renderer-windows", names)
                self.assertNotIn("renderer-dx12", names)

    def test_native_backend_does_not_depend_on_selector_vulkan_or_ui(self):
        names = [path.name for path in PACKER.package_dependencies(
            ROOT / "packages/renderer-dx12", "windows")]
        self.assertIn("renderer", names)
        self.assertFalse({"renderer-windows", "renderer-vulkan", "ui"} & set(names))


if __name__ == "__main__":
    unittest.main()
