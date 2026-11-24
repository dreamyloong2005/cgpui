"""Focused source ownership/security guards, not remote-policy or GPU evidence."""
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[3]


class WindowsDx12Structure(unittest.TestCase):
    def text(self, relative):
        return (ROOT / relative).read_text(encoding="utf-8")

    def test_focused_leaf_owners_stay_small(self):
        caps = {
            "scripts/ci/windows_dx12_inventory.py": 125,
            "scripts/ci/windows_dx12_log.py": 140,
            "scripts/ci/windows_dx12_evidence.py": 100,
            "scripts/ci/windows_dx12_desktop.cs": 90,
            "scripts/ci/windows-dx12-prerequisites.ps1": 130,
            "scripts/ci/windows-dx12-run.py": 120,
        }
        for path, cap in caps.items():
            with self.subTest(path=path):
                self.assertLessEqual(len(self.text(path).splitlines()), cap)

    def test_workflows_are_manual_and_default_branch_only(self):
        for name in ("common", "desktop"):
            text = self.text(f".github/workflows/dx12-warp-{name}.yml")
            with self.subTest(workflow=name):
                self.assertIn("  workflow_dispatch:", text)
                self.assertNotRegex(text, r"(?m)^\s+(push|pull_request|pull_request_target|workflow_run|schedule):")
                self.assertIn('refs/heads/$env:DEFAULT_BRANCH', text)
                self.assertIn("contents: read", text)
                self.assertNotIn("continue-on-error:", text)
                self.assertNotIn("secrets.", text)

    def test_hosted_source_is_pinned_and_fixtures_have_an_owner(self):
        text = self.text(".github/workflows/dx12-warp-common.yml")
        self.assertIn("runs-on: windows-2025", text)
        self.assertIn("ref: ${{ github.sha }}", text)
        self.assertIn("persist-credentials: false", text)
        self.assertIn("discover -s scripts/ci/dx12_fixtures", text)

    def test_desktop_cannot_schedule_an_unauthorized_runner(self):
        text = self.text(".github/workflows/dx12-warp-desktop.yml")
        self.assertEqual(text.count("runs-on:"), 1)
        self.assertIn("runs-on: windows-2025", text)
        self.assertNotIn("actions/checkout", text)
        self.assertNotIn("runs-on:\n", text)
        self.assertNotIn("vars.", text)
        self.assertIn("authorized=$false; native_execution=$false", text)
        self.assertIn("throw 'Desktop CI is unprovisioned:", text)

    def test_lane_does_not_invent_a_fixed_suite_total(self):
        text = self.text("scripts/ci/windows_dx12_inventory.py")
        self.assertIn("windows_tests(root, suite)", text)
        self.assertIn('"manifest_sha256": manifests', text)
        self.assertNotIn("windows-dx12-inventory.json", text)
        self.assertFalse((ROOT / "scripts/ci/windows-dx12-inventory.json").exists())

    def test_native_selection_and_execution_remain_explicit(self):
        text = self.text("scripts/ci/windows-dx12-run.py")
        self.assertIn('CGPUI_DX12_ADAPTER="warp"', text)
        self.assertIn('CGPUI_DX12_DEBUG="1"', text)
        self.assertIn('CGPUI_DX12_GPU_VALIDATION="1"', text)
        self.assertIn('environment["MCPP_JOBS"] = "4"', text)
        self.assertIn('environment["MCPP_JOBS"] = "1"', text)
        self.assertIn("check_both(", text)
        self.assertIn("before != after", text)


if __name__ == "__main__":
    unittest.main()
