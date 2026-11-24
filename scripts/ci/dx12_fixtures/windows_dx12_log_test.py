"""Synthetic output fixtures: these tests do not create a device or prove WARP."""
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from windows_dx12_log import check_both, check_run


CASE = "renderer_pixel_backend_identity_test"
SECOND = "renderer_image_pixel_test"


def identity(renderer="dx12"):
    return (f"pixel renderer: requested={renderer} expected={renderer} actual={renderer} "
            f"selection={2 if renderer == 'dx12' else 1} adapter=Synthetic Fixture Device "
            f"vendor=123 luid=456 software={1 if renderer == 'dx12' else 0} fallback=\n")


def output(names=(CASE,), renderer="dx12"):
    return (f"CGPUI test run: renderer={renderer}\n" +
            "".join(f"{name} ... ok (0.10s)\n{identity(renderer)}\n" for name in names) +
            f" test result ok. {len(names)} passed; 0 failed; finished in 1.0s\n")


class WindowsDx12Log(unittest.TestCase):
    def test_exact_success(self):
        result = check_run(output(), [CASE], "dx12", [CASE])
        self.assertEqual(result["test_count"], 1)
        self.assertEqual(result["identity_reports"], 1)

    def test_zero_matching_tests_is_failure(self):
        with self.assertRaises(ValueError):
            check_run(output(()), [CASE], "dx12", [CASE])

    def test_missing_extra_duplicate_names_fail(self):
        for names in ((SECOND,), (CASE, SECOND), (CASE, CASE)):
            with self.subTest(names=names), self.assertRaises(ValueError):
                check_run(output(names), [CASE], "dx12", [CASE])

    def test_duplicate_expected_inventory_is_not_a_pass(self):
        with self.assertRaises(ValueError):
            check_run(output((CASE, CASE)), [CASE, CASE], "dx12", [CASE])

    def test_named_failure_rejected_even_with_success_summary(self):
        with self.assertRaises(ValueError):
            check_run(output().replace("... ok", "... FAIL"), [CASE], "dx12", [CASE])

    def test_missing_duplicate_and_inconsistent_summary_fail(self):
        summary = " test result ok. 1 passed; 0 failed; finished in 1.0s\n"
        for suffix in ("", summary * 2, summary.replace("1 passed", "2 passed"),
                       summary.replace("0 failed", "1 failed"), summary.replace("ok.", "FAILED.")):
            with self.subTest(suffix=suffix), self.assertRaises(ValueError):
                check_run(output().replace(summary, suffix), [CASE], "dx12", [CASE])

    def test_no_run_build_output_cannot_count(self):
        with self.assertRaises(ValueError):
            check_run("CGPUI test run: renderer=dx12\n test result ok. 0 passed; 0 failed; 1 built, not run\n",
                      [CASE], "dx12", [CASE])

    def test_skips_fail_even_on_exit_zero_success_summary(self):
        for skip in ("SKIP ordinary alpha positive cases", "SKIP: no independent SM6 adapter", "skipped"):
            with self.subTest(skip=skip), self.assertRaises(ValueError):
                check_run(output() + skip, [CASE], "dx12", [CASE])

    def test_identity_is_required_per_case_not_per_log(self):
        value = output((CASE, SECOND)).replace(identity(), "", 1)
        with self.assertRaises(ValueError):
            check_run(value, [CASE, SECOND], "dx12", [CASE, SECOND])

    def test_malformed_and_wrong_identity_fail(self):
        for before, after in (("actual=dx12", "actual=vulkan"), ("selection=2", "selection=0"),
                              ("software=1", "software=0"), ("luid=456", "luid=unavailable"),
                              ("luid=456", "luid=0"), ("fallback=", "fallback=unexpected"),
                              ("vendor=123", "vendor=unknown"), ("expected=dx12", "expected=vulkan"),
                              ("adapter=Synthetic Fixture Device", "adapter=  ")):
            with self.subTest(after=after), self.assertRaises(ValueError):
                check_run(output().replace(before, after), [CASE], "dx12", [CASE])

    def test_all_identity_reports_must_match(self):
        value = output().replace(identity(), identity() + identity().replace("software=1", "software=0"))
        with self.assertRaises(ValueError):
            check_run(value, [CASE], "dx12", [CASE])

    def test_wrong_missing_duplicate_marker_fail(self):
        marker = "CGPUI test run: renderer=dx12\n"
        for replacement in ("", marker * 2, marker.replace("dx12", "both")):
            with self.subTest(marker=replacement), self.assertRaises(ValueError):
                check_run(output().replace(marker, replacement), [CASE], "dx12", [CASE])

    def test_single_run_marker_cannot_follow_complete_results(self):
        marker = "CGPUI test run: renderer=dx12\n"
        # Same exact valid case/identity/summary; only the ownership marker moves.
        value = output().replace(marker, "") + marker
        with self.assertRaises(ValueError):
            check_run(value, [CASE], "dx12", [CASE])

    def test_single_run_marker_cannot_follow_its_case_prefix(self):
        marker = "CGPUI test run: renderer=dx12\n"
        case = f"{CASE} ... ok (0.10s)\n"
        value = output().replace(marker + case, case + marker)
        with self.assertRaises(ValueError):
            check_run(value, [CASE], "dx12", [CASE])

    def test_single_run_marker_allows_ordinary_prefix_build_chatter(self):
        value = "   Workspace building member 'cgpui.tests-renderer'\n    Finished dev in 1.0s\n" + output()
        result = check_run(value, [CASE], "dx12", [CASE])
        self.assertEqual(result["test_count"], 1)
        self.assertEqual(result["identity_reports"], 1)

    def test_ansi_and_crlf_are_normalized(self):
        result = check_run("\x1b[32m" + output().replace("\n", "\r\n") + "\x1b[0m", [CASE], "dx12", [CASE])
        self.assertEqual(result["identity_reports"], 1)

    def test_both_requires_two_complete_ordered_runs(self):
        value = output(renderer="vulkan") + output()
        self.assertEqual([r["renderer"] for r in check_both(value, [CASE], [CASE])], ["vulkan", "dx12"])
        for wrong in (output(), output() + output(renderer="vulkan"), value + output()):
            with self.subTest(wrong=wrong), self.assertRaises(ValueError):
                check_both(wrong, [CASE], [CASE])

    def test_last_summary_cannot_hide_first_backend_failure(self):
        value = output(renderer="vulkan").replace("... ok", "... FAIL") + output()
        with self.assertRaises(ValueError):
            check_both(value, [CASE], [CASE])

    def test_both_checks_prefix_skip_before_slicing(self):
        value = output(renderer="vulkan") + output()
        for prefix in ("SKIP: unavailable first probe\n", "\x1b[33mSKIPPED\x1b[0m\r\n"):
            with self.subTest(prefix=prefix), self.assertRaises(ValueError):
                check_both(prefix + value, [CASE], [CASE])

    def test_both_rejects_unowned_prefix_case_and_summary(self):
        value = output(renderer="vulkan") + output()
        for prefix in ("foreign_test ... ok (0.1s)\n", "foreign_test ... FAIL (0.1s)\n",
                       " test result ok. 1 passed; 0 failed; finished in 0.1s\n"):
            with self.subTest(prefix=prefix), self.assertRaises(ValueError):
                check_both(prefix + value, [CASE], [CASE])

    def test_both_allows_ordinary_prefix_build_chatter(self):
        value = "   Workspace building member 'cgpui.tests-renderer'\n    Finished dev in 1.0s\n"
        value += output(renderer="vulkan") + output()
        self.assertEqual(len(check_both(value, [CASE], [CASE])), 2)


if __name__ == "__main__":
    unittest.main()
