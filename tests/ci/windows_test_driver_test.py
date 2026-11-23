"""Behavior of scripts/dev/test.ps1 through its mock command boundary."""

import sys
import unittest
from windows_test_driver_support import POWERSHELL, ROOT, run_driver


@unittest.skipUnless(sys.platform == "win32" and POWERSHELL, "requires Windows PowerShell")
class WindowsTestDriver(unittest.TestCase):
    def test_no_selector_is_one_run_and_preserves_inherited_environment(self):
        for environment, expected in (({}, None), ({"CGPUI_RENDERER": "dx12"}, "dx12")):
            with self.subTest(environment=environment):
                process, calls, state, _ = run_driver(["-p", "tests/renderer"], **environment)
                self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
                self.assertEqual(len(calls), 1)
                self.assertEqual(calls[0]["renderer"], expected)
                self.assertEqual(state["before"], state["after"])

    def test_both_runs_same_arguments_with_two_named_backends(self):
        args = ["-p", "tests/renderer", "pixel filter", "--export", "result dir", "-Renderer", "both"]
        process, calls, state, repo = run_driver(args, MCPP_EXPORT="export sentinel")
        self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
        self.assertEqual([call["renderer"] for call in calls], ["vulkan", "dx12"])
        for call in calls:
            self.assertEqual(call["arguments"], ["test", *args[:-2]])
            self.assertEqual(call["cwd"], str(repo))
            self.assertEqual(call["compile_commands"], "compile_commands.json")
            self.assertEqual(call["export"], "export sentinel")
        self.assertEqual(state["before"], state["after"])

    def test_explicit_auto_and_named_selectors_remain_single_runs(self):
        for renderer in ("auto", "vulkan", "dx12"):
            with self.subTest(renderer=renderer):
                process, calls, state, _ = run_driver(
                    ["-Renderer", renderer, "-p", "tests/renderer", "pixel filter"],
                    CGPUI_RENDERER="inherited", MCPP_COMPILE_COMMANDS="custom commands.json")
                self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
                self.assertEqual(len(calls), 1)
                self.assertEqual(calls[0]["renderer"], renderer)
                self.assertEqual(calls[0]["arguments"], ["test", "-p", "tests/renderer", "pixel filter"])
                self.assertEqual(calls[0]["compile_commands"], "custom commands.json")
                self.assertEqual(state["before"], state["after"])

    def test_powershell_colon_and_case_insensitive_selector_spelling(self):
        for selector in (["-Renderer:dx12"], ["-renderer", "dx12"]):
            with self.subTest(selector=selector):
                process, calls, state, _ = run_driver(selector)
                self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
                self.assertEqual([call["renderer"] for call in calls], ["dx12"])
                self.assertEqual(calls[0]["arguments"], ["test"])
                self.assertEqual(state["before"], state["after"])

    def test_absent_mcpp_home_is_restored(self):
        process, calls, state, _ = run_driver(["-Renderer", "both"], MCPP_HOME=None)
        self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
        self.assertEqual(len(calls), 2)
        self.assertIsNone(state["before"]["home"])
        self.assertEqual(state["before"], state["after"])

    def test_strict_mode_preserves_native_preference_presence_and_two_runs(self):
        process, calls, state, _ = run_driver(
            ["-Renderer", "both"], MOCK_STRICT_MODE="true", MOCK_NATIVE="1",
            MOCK_EXIT_vulkan="31", MOCK_EXIT_dx12="0")
        self.assertEqual(process.returncode, 31, process.stdout + process.stderr)
        self.assertEqual([call["renderer"] for call in calls], ["vulkan", "dx12"])
        if state["host"]["version"].startswith("5."):
            self.assertIs(state["before"]["native_preference_exists"], False)
            self.assertIsNone(state["before"]["native_preference"])
        self.assertEqual(state["before"], state["after"])

    def test_no_arguments_still_invokes_mcpp_once(self):
        process, calls, state, _ = run_driver([])
        self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
        self.assertEqual([call["arguments"] for call in calls], [["test"]])
        self.assertEqual(state["before"], state["after"])

    def test_invalid_missing_repeated_and_inline_selectors_fail_before_invocation(self):
        cases = [(["-Renderer"], "must be"),
                 (["-Renderer", "-p", "tests/renderer"], "must be"),
                 (["-Renderer", "metal"], "must be"),
                 (["-Renderer", "DX12"], "must be"),
                 (["-Renderer", "vulkan", "-Renderer", "dx12"], "only be specified once"),
                 (["-Renderer", "both", "-renderer", "auto"], "only be specified once"),
                 (["-Renderer", "-Renderer", "auto"], "only be specified once"),
                 (["-Renderer=both"], "separate value")]
        for args, message in cases:
            with self.subTest(arguments=args):
                process, calls, state, _ = run_driver(args, CGPUI_RENDERER="inherited")
                self.assertNotEqual(process.returncode, 0)
                self.assertIn(message, process.stdout + process.stderr)
                self.assertEqual(calls, [])
                self.assertEqual(state["before"], state["after"])

    def test_both_propagates_either_failure_and_keeps_the_first_failure(self):
        for vulkan, dx12, expected in ((21, 0, 21), (0, 22, 22), (21, 22, 21)):
            with self.subTest(vulkan=vulkan, dx12=dx12):
                process, calls, state, _ = run_driver(
                    ["-Renderer", "both", "filter"], CGPUI_RENDERER="original",
                    MOCK_EXIT_vulkan=str(vulkan), MOCK_EXIT_dx12=str(dx12))
                self.assertEqual(process.returncode, expected, process.stdout + process.stderr)
                self.assertEqual([call["renderer"] for call in calls], ["vulkan", "dx12"])
                self.assertEqual(state["before"], state["after"])

    def test_single_run_failure_is_propagated(self):
        for selector in ([], ["-Renderer", "auto"], ["-Renderer", "dx12"]):
            with self.subTest(selector=selector):
                process, calls, state, _ = run_driver(selector, MOCK_EXIT_DEFAULT="19")
                self.assertEqual(process.returncode, 19)
                self.assertEqual(len(calls), 1)
                self.assertEqual(state["before"], state["after"])

    def test_entry_and_private_parser_remain_focused(self):
        entry = (ROOT / "scripts/dev/test.ps1").read_text("utf-8")
        parser = (ROOT / "scripts/dev/test-renderer.ps1").read_text("utf-8")
        self.assertLessEqual(len(entry.splitlines()), 95)
        self.assertLessEqual(len(parser.splitlines()), 40)
        self.assertIn("'test-renderer.ps1'", entry)
        self.assertNotIn("function Get-CgpuiTestRendererPlan", entry)
        self.assertNotIn("& $Mcpp", parser)


if __name__ == "__main__":
    unittest.main()
