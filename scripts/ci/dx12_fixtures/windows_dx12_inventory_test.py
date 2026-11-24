"""Manifest fixtures: no build, child process or native GPU execution."""
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from windows_dx12_inventory import load_inventory, windows_tests


class WindowsDx12Inventory(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.member = self.root / "tests" / "renderer"
        self.member.mkdir(parents=True)
        self.manifest = self.member / "mcpp.toml"
        for name in ("neutral_test", "windows_test", "linux_test", "both_test"):
            (self.member / (name + ".cpp")).write_text("// Synthetic fixture\n")
        self.text = ('[test]\ndiscover=["neutral_test.cpp"]\n'
                     '[target.\'cfg(os = "windows")\'.test]\ndiscover=["windows_test.cpp"]\n'
                     '[target.\'cfg(os = "linux")\'.test]\ndiscover=["linux_test.cpp"]\n'
                     '[target.\'cfg(any(os = "windows", os = "linux"))\'.test]\ndiscover=["both_test.cpp"]\n')
        self.manifest.write_text(self.text)

    def test_known_conditions_exact_union(self):
        self.assertEqual(windows_tests(self.root, "renderer"), ["both_test", "neutral_test", "windows_test"])

    def test_unknown_test_condition_fails_closed(self):
        self.manifest.write_text(self.text + '[target.\'cfg(arch = "x86_64")\'.test]\ndiscover=[]\n')
        with self.assertRaises(ValueError):
            windows_tests(self.root, "renderer")

    def test_glob_and_parent_traversal_require_review(self):
        for name in ("*_test.cpp", "../neutral_test.cpp", "/tmp/neutral_test.cpp"):
            self.manifest.write_text(self.text.replace("windows_test.cpp", name))
            with self.subTest(name=name), self.assertRaises(ValueError):
                windows_tests(self.root, "renderer")

    def test_missing_source_and_duplicate_names_fail(self):
        for name in ("missing_test.cpp", "neutral_test.cpp"):
            self.manifest.write_text(self.text.replace("windows_test.cpp", name))
            with self.subTest(name=name), self.assertRaises(ValueError):
                windows_tests(self.root, "renderer")

    def test_new_manifest_registration_is_derived_not_a_stale_total(self):
        (self.member / "new_test.cpp").write_text("// Added after earlier inventory\n")
        self.manifest.write_text(self.text.replace('"windows_test.cpp"', '"windows_test.cpp", "new_test.cpp"'))
        self.assertEqual(windows_tests(self.root, "renderer"),
                         ["both_test", "neutral_test", "new_test", "windows_test"])

    def test_real_inventory_uses_current_owner_and_manifest_hashes(self):
        root = Path(__file__).resolve().parents[3]
        inventory = load_inventory(root)
        self.assertEqual(inventory["version"], 2)
        self.assertEqual(inventory["common"][0], "renderer_pixel_backend_identity_test")
        for suite, names in inventory["suites"].items():
            self.assertEqual(names, windows_tests(root, suite))
            self.assertRegex(inventory["manifest_sha256"][f"tests/{suite}/mcpp.toml"], r"^[a-f0-9]{64}$")
        self.assertTrue(set(inventory["common"]) <= set(inventory["suites"]["renderer"]))

    def test_missing_common_scene_fails_before_execution(self):
        root = Path(__file__).resolve().parents[3]
        with patch("windows_dx12_inventory.COMMON", ("renderer_pixel_backend_identity_test", "absent_scene")):
            with self.assertRaises(ValueError):
                load_inventory(root)


if __name__ == "__main__":
    unittest.main()
