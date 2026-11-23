# Independent Standards review

Date: 2026-11-24. Baseline: `36b3cd40`, branch `phase-v`.
Scope: `git diff 36b3cd40` plus the new untracked common-pixel, runtime-recovery,
driver and fixture files; excluded user `.vscode/`. The reviewed source includes
the final applied StrictMode-safe driver and Windows fixture CI registration.

## Result

**0 confirmed actionable findings.** No documented-standard violation or
actionable Fowler-baseline smell was identified in this slice.

- `AGENTS.md`'s module-first ownership is respected: HWND hosting, selection,
  capture, pure pixel probes, mathematical references and runtime observation
  have focused implementation leaves. The common support header contains the
  necessary templates and declarations rather than native implementation.
- New Windows-only support/dependencies stay platform-scoped in mcpp. Portable
  shadow/text/probe implementations do not pull in native window headers; Metal
  keeps its same mathematical reference through the neutral helper. Existing
  scene-cap/oracle guards are migrated, not discarded; new ownership guards
  are registered in both mcpp and the CI architecture target inventory.
- The runtime Renderer forwarding observer is an intentional test seam, not an
  actionable Middle Man: it returns the same native frame and preserves native
  reporting while observing actual retries. The owning scene retains its image
  data, and loop observations outlive runtime-owned renderers.
- The PowerShell parser and runner remain separate focused modules. Ordinary
  native exit results are aggregated without swallowing setup/invocation
  exceptions; preference, environment and location cleanup has clear ownership.
  The StrictMode-safe preference read and native-command fixture preserve this
  boundary. ASan preparation remains outside the repeated backend loop.
- CI invokes the isolated fixtures under both Windows PowerShell hosts and
  checks each Python exit. User documentation distinguishes scene availability,
  historical results and local acceptance from full/backend-wide readiness.

## Limits

This was an independent read-only source/standards review; no project builds,
tests or GPU probes were run by this reviewer. Root owns behavioral and matrix
acceptance. Purely tooling-enforced limits were not duplicated as findings.
No claim is made that hosted CI or native macOS validation has run.

## Follow-up: real short-path TEMP regression

Root's later default-TEMP fixture execution exposed a real 8.3-alias mismatch:
PowerShell's canonical paths differed textually from Python's unnormalized
expected paths. Earlier source review did not detect this environment-dependent
fixture defect; the original failed matrix remains evidence and is not green.

Reviewed the two-file correction after root restored the temporary RED mutation:
`windows_test_driver_support.py` canonicalizes the newly created directory with
`Path(...).resolve()`, and `windows_test_driver_short_path_test.py` constructs a
real Windows alias using `GetShortPathNameW`, verifies `samefile`, then feeds that
actual alias through the existing fixture seam. Exact path/argument/environment
assertions remain intact for both ordinary and ASan runs. A volume without a
distinct alias produces an explicit skip rather than fabricated short names.

**0 confirmed actionable findings in the correction.** Ownership stays in the
focused fixture support and regression file; production scripts are unchanged.
This follow-up is static review only. Root owns RED/GREEN execution and the new
freeze/full-matrix rerun; the earlier frozen source is not represented as the
corrected accepted source.
