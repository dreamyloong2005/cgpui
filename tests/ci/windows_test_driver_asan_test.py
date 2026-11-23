"""ASan forwarding uses the same isolated command boundary as the driver tests."""

import sys
import unittest
from windows_test_driver_support import POWERSHELL, run_driver


@unittest.skipUnless(sys.platform == "win32" and POWERSHELL, "requires Windows PowerShell")
class WindowsTestDriverAsan(unittest.TestCase):
    def test_both_prepares_once_and_translates_each_member_path_once(self):
        for switches, directory, symbols in ((["-Asan"], "asan", []),
                                              (["-AsanSymbols"], "asan-symbols", ["--symbols"]),
                                              (["-Asan", "-AsanSymbols"], "asan-symbols", ["--symbols"])):
            with self.subTest(switches=switches):
                args = [*switches, "-p", "tests/renderer", "-Renderer", "both", "-p",
                        "../../tests/ui", "-p", "tests/runtime", "pixel filter"]
                process, calls, state, repo = run_driver(args, MCPP_EXPORT="export sentinel")
                self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
                self.assertEqual([call["kind"] for call in calls], ["python", "cl", "mcpp", "mcpp"])
                workdir = repo / "build" / directory
                self.assertEqual(calls[0]["arguments"], [str(repo / "scripts/dev/asan_workspace.py"),
                                                        "windows", str(workdir), *symbols])
                self.assertIn("/Fo" + str(workdir / "asan_entry.obj"), calls[1]["arguments"])
                self.assertIn("/MD", calls[1]["arguments"])
                self.assertEqual([call["renderer"] for call in calls[2:]], ["vulkan", "dx12"])
                for call in calls[2:]:
                    self.assertEqual(call["arguments"], ["test", "-p", "../../tests/renderer", "-p",
                                                         "../../tests/ui", "-p", "../../tests/runtime",
                                                         "pixel filter"])
                    self.assertEqual(call["cwd"], str(workdir))
                    self.assertTrue(call["path"].startswith(str(repo / "asan runtime") + ";"))
                    self.assertIsNone(call["compile_commands"])
                    self.assertEqual(call["export"], "export sentinel")
                self.assertEqual(state["before"], state["after"])

    def test_existing_compile_commands_are_forwarded_under_asan(self):
        process, calls, state, _ = run_driver(["-Asan", "-Renderer", "auto"],
                                              MCPP_COMPILE_COMMANDS="explicit.json")
        self.assertEqual(process.returncode, 0, process.stdout + process.stderr)
        self.assertEqual(calls[-1]["renderer"], "auto")
        self.assertEqual(calls[-1]["compile_commands"], "explicit.json")
        self.assertEqual(state["before"], state["after"])

    def test_asan_setup_failure_does_not_invoke_mcpp_and_restores_environment(self):
        for environment, kinds, code in (({"MOCK_PYTHON_EXIT": "17"}, ["python"], 17),
                                          ({"MOCK_CL_EXIT": "23"}, ["python", "cl"], 1)):
            with self.subTest(environment=environment):
                process, calls, state, _ = run_driver(["-Asan", "-Renderer", "both"], **environment)
                self.assertEqual(process.returncode, code, process.stdout + process.stderr)
                self.assertEqual([call["kind"] for call in calls], kinds)
                self.assertEqual(state["before"], state["after"])


if __name__ == "__main__":
    unittest.main()
