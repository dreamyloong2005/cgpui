"""Synthetic process-boundary fixtures; no child commands or graphics are run."""
from pathlib import Path
import json
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from windows_dx12_evidence import run_command


class WindowsDx12Evidence(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def test_nonzero_exit_never_becomes_success(self):
        with patch("windows_dx12_evidence.subprocess.run", return_value=subprocess.CompletedProcess([], 37)):
            with self.assertRaises(RuntimeError):
                run_command(self.root, self.root, "failed", ["synthetic command"], {})
        value = json.loads((self.root / "failed.receipt.json").read_text())
        self.assertEqual(value["exit_code"], 37)
        self.assertGreaterEqual(value["elapsed_seconds"], 0)

    def test_launch_failure_retains_null_exit_and_reason(self):
        with patch("windows_dx12_evidence.subprocess.run", side_effect=OSError("synthetic launch failure")):
            with self.assertRaises(OSError):
                run_command(self.root, self.root, "launch", ["synthetic command"], {})
        value = json.loads((self.root / "launch.receipt.json").read_text())
        self.assertIsNone(value["exit_code"])
        self.assertIn("synthetic launch failure", value["launch_error"])

    def test_existing_log_is_never_overwritten(self):
        (self.root / "first.log").write_text("original failed evidence")
        with patch("windows_dx12_evidence.subprocess.run") as process:
            with self.assertRaises(ValueError):
                run_command(self.root, self.root, "first", ["synthetic command"], {})
            process.assert_not_called()
        self.assertEqual((self.root / "first.log").read_text(), "original failed evidence")

    def test_success_preserves_wrapper_argv_and_only_returns_captured_log(self):
        def fake(command, **kwargs):
            self.assertEqual(command[:2], ["bash", "scripts/ci/run-with-github-annotation.sh"])
            self.assertEqual(command[3:], ["--", "pwsh", "-File", "argument with spaces"])
            self.assertEqual(kwargs["env"], {"SENTINEL": "kept"})
            (self.root / "ok.log").write_text("synthetic complete stdout and stderr")
            return subprocess.CompletedProcess(command, 0)
        with patch("windows_dx12_evidence.subprocess.run", side_effect=fake):
            value = run_command(self.root, self.root, "ok", ["pwsh", "-File", "argument with spaces"],
                                {"SENTINEL": "kept"})
        self.assertEqual(value, "synthetic complete stdout and stderr")


if __name__ == "__main__":
    unittest.main()
