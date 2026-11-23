# Progress

Date: 2026-11-23 (Asia/Shanghai). Baseline c40dd0c4, phase-v.

- Strict stable native-menu feedback loop: uncorrected red; alpha-only repair
  green with no relaxed RGB tolerances. GdiFlush before SetAlpha is necessary.
- Event-driven native menu loop, real WM_COMMAND, Escape cancellation validated.
- Stale prior HWND-flip content repro kept red; boundary intentionally initial-only.
- Focused platform/DX12 modules implemented, no public modes or broadened caps.
- Pointer protocol replaced by scalar instance/cookie messages; actual malformed
  PostMessage regression includes zero WPARAM at the former pointer entry.
- Cross-thread opaque factory regression reproduced (opaque-thread-red.log), fixed
  by dedicated bounded history exchange, green in all 11 final ordinary cases.
- Older-OS gate does not skip failed initialization on supported builds.
- Probes archived separately; no known-negative test remains in production suite.
- Three docs updated with bounded support and explicit unverified cases.
- Review Standards: no hard violation; CI matrix registration omission fixed.
  One nonblocking duplicate test-comparison heuristic retained for per-scenario
  readable diagnostics. Spec: both actionable compatibility issues closed.
- Full frozen-source validation started; acceptance record pending final logs.

- First full Windows run: all behavior suites including 64 DX12 tests passed,
  but five pre-existing architecture checks caught a shared 134-line header cap
  (new leaf include/member had made it 136). Two short existing method
  declarations were normalized to the surrounding single-line style, restoring
  134 without moving implementation or changing a cap. The new alpha structure
  test now explicitly guards that existing cap too. Full source re-freeze and
  repeat validation required; previous failure retained in
  windows-structure-cap-red.log.

- Final frozen driver exited0: Windows732/0 and all-member build; WSL640/0 and
  all-member build; Intel/NVIDIA/WARP native64/0 each; ASan64/0; SDK fixtures4+15;
  installed Windows SDK compile/link/run including native ordinary alpha.
- Windows raw-byte and WSL normalized manifests match all2758 frozen files.
- D: ended at about341MiB free after the retained installed-SDK copies. Scoped
  cleanup of only those new copies was denied before execution; no cleanup
  performed and no pre-existing user/build data touched.
