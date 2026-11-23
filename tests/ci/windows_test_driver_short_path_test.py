"""Real Windows short-path aliases must not change fixture path expectations."""

import ctypes
from ctypes import wintypes
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

from windows_test_driver_support import POWERSHELL, run_driver


def short_alias(path):
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    get_short_path = kernel32.GetShortPathNameW
    get_short_path.argtypes = [wintypes.LPCWSTR, wintypes.LPWSTR, wintypes.DWORD]
    get_short_path.restype = wintypes.DWORD
    required = get_short_path(str(path), None, 0)
    if not required:
        raise ctypes.WinError(ctypes.get_last_error())
    buffer = ctypes.create_unicode_buffer(required)
    length = get_short_path(str(path), buffer, required)
    if not length or length >= required:
        raise ctypes.WinError(ctypes.get_last_error())
    return Path(buffer.value)


@unittest.skipUnless(sys.platform == "win32" and POWERSHELL, "requires Windows PowerShell")
class WindowsTestDriverShortPath(unittest.TestCase):
    def test_real_short_temp_path_matches_normal_and_asan_invocation_paths(self):
        for asan in (False, True):
            with self.subTest(asan=asan):
                # Use the actual system TEMP, not a replacement drive or a fake alias.
                directory = Path(tempfile.mkdtemp(prefix="cgpui driver long path ")).resolve()
                alias = short_alias(directory)
                self.assertTrue(alias.samefile(directory))
                if os.path.normcase(str(alias)) == os.path.normcase(str(directory)):
                    self.skipTest("the system TEMP volume exposes no distinct Windows short-path alias")
                args = (["-Asan"] if asan else []) + ["-Renderer", "both", "-p", "tests/renderer", "pixel filter"]
                with mock.patch("windows_test_driver_support.tempfile.mkdtemp", return_value=str(alias)):
                    process, calls, state, repo = run_driver(args)
                self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
                expected_repo = directory / "fake repo"
                self.assertEqual(repo, expected_repo)
                expected_cwd = expected_repo / "build/asan" if asan else expected_repo
                if asan:
                    self.assertEqual([call["kind"] for call in calls], ["python", "cl", "mcpp", "mcpp"])
                    self.assertEqual(calls[0]["arguments"], [str(expected_repo / "scripts/dev/asan_workspace.py"),
                                                            "windows", str(expected_cwd)])
                    self.assertIn("/Fo" + str(expected_cwd / "asan_entry.obj"), calls[1]["arguments"])
                    runs = calls[2:]
                else:
                    self.assertEqual([call["kind"] for call in calls], ["mcpp", "mcpp"])
                    runs = calls
                self.assertEqual([call["renderer"] for call in runs], ["vulkan", "dx12"])
                for call in runs:
                    self.assertEqual(call["cwd"], str(expected_cwd))
                    self.assertEqual(call["arguments"], ["test", "-p",
                                                         "../../tests/renderer" if asan else "tests/renderer",
                                                         "pixel filter"])
                    if asan:
                        self.assertTrue(call["path"].startswith(str(expected_repo / "asan runtime") + ";"))
                self.assertEqual(state["before"], state["after"])


if __name__ == "__main__":
    unittest.main()
