"""Native exit codes must not become PowerShell preference-driven exceptions."""

import unittest
from windows_test_driver_support import HOST_DETAILS, run_driver


@unittest.skipUnless(HOST_DETAILS.get("native_exit_preference"), "requires PS7 native error preference")
class WindowsTestDriverNative(unittest.TestCase):
    def test_both_handles_native_failures_even_when_caller_enables_native_errors(self):
        for vulkan, dx12, expected in ((21, 0, 21), (0, 22, 22), (21, 22, 21)):
            with self.subTest(vulkan=vulkan, dx12=dx12):
                process, calls, state, _ = run_driver(
                    ["-Renderer", "both", "-p", "tests/renderer", "pixel filter"],
                    MOCK_NATIVE="1", MOCK_NATIVE_PREFERENCE="true", CGPUI_RENDERER="original",
                    MOCK_EXIT_vulkan=str(vulkan), MOCK_EXIT_dx12=str(dx12))
                self.assertEqual(process.returncode, expected, process.stdout + process.stderr)
                self.assertEqual([call["renderer"] for call in calls], ["vulkan", "dx12"])
                for call in calls:
                    self.assertEqual(call["arguments"], ["test", "-p", "tests/renderer", "pixel filter"])
                self.assertIs(state["before"]["native_preference"], True)
                self.assertEqual(state["before"], state["after"])

    def test_invocation_exception_remains_fail_fast_and_restores_caller_state(self):
        process, calls, state, _ = run_driver(["-Renderer", "both"],
                                              MOCK_NATIVE_PREFERENCE="true", MOCK_THROW="vulkan")
        self.assertNotEqual(process.returncode, 0)
        self.assertIn("mock invocation exception", process.stdout + process.stderr)
        self.assertEqual([call["renderer"] for call in calls], ["vulkan"])
        self.assertIs(state["before"]["native_preference"], True)
        self.assertEqual(state["before"], state["after"])


if __name__ == "__main__":
    unittest.main()
