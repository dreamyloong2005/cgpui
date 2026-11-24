# Actual-source Standards review — steady-state benchmark

2026-11-24. Reviewer: `/root/capture_standards_review/orphan_test_review`.

## Scope

Baseline/HEAD: `d794f1145e339ed2c459d2bc69659e2dd236c0ae`; no intervening commits.
Reviewed `git diff HEAD` (three registration additions) plus25 untracked benchmark
files; excluded `.vscode`. Normative sources: root `AGENTS.md`, this directory's
`task_plan.md`, external `revised-creation` and `reproducible-tools` deliveries.
After the source-backed batch-documentation clarification,23/25 new files match
the corresponding delivery after CRLF/LF normalization. The two semantic
deviations are the architecture guard's section-search anchors and the documented
backend diagnostic-batch convention; measurement sources remain unchanged.
Actual guard SHA256:
`dd2d4fbd1f951bf3d9f2fca588c55e5c189e208211eebb3223bcdd57875a2a4f`.

## Historical finding — closed at source level

**[P2, closed] Preserve CRLF portability when anchoring the manifest section** —
`tests/architecture/renderer_steady_state_structure_test.cpp:81` previously searched
`"\n[test]\n"`, while `read()` uses ordinary `ifstream` without newline
normalization. The actual manifest has47 CRLF line endings. Windows text-mode
translation hides this, but Linux/WSL reading the same bytes retains `\r`, so the
new guard falsely rejects an otherwise valid member. Root AGENTS requires module
structure to stay green; the task plan includes Windows/WSL regression. The
leading-newline anchor is sufficient to exclude the first comment's `[test]`;
remove the required trailing newline or explicitly support CRLF. Independently
reread the fix: begin now uses `"\n[test]"`; end remains `"\n[target."`.
Both newline styles now identify the real section without matching the comment;
all exclusion/discovery assertions remain intact. Root's external
`actual/section-fixture-results.json` records LF/CRLF acceptance and manual-bin
injection rejection2/2. Those raw-section fixtures are not C++ test execution.

## Otherwise conforming

No other actionable documented-standard violation or heuristic smell found.
Dedicated scene/acquisition/window/display/run/statistics/format/IO owners keep
production renderer and platform entry files untouched. Private headers remain
declarations/data; largest execution owner is134lines, below its140line guard.
Manual native bin is outside discovery; four pure regressions and the structure
guard are registered once, preserving existing registry entries. No xmake or
public policy/API is introduced.

Runner and helper remain focused, owned-process and new-directory-only; timeout,
child exit and evidence-IO failure are distinct. Fixture launcher adaptation
preserves the process/failure suffix. Documentation separates labels from actual
release flags/provenance, creation/cold/steady envelopes, synthetic/native
evidence and measurements from speedup/GPU-time claims.

The added batch paragraph was separately reviewed against Vulkan presentation's
five-span planner call, `planning/report_commands.cpp:53-68` (empty image span),
Vulkan's separate submitted-image/draw accounting, and DX12's visible-group
diagnostics. It correctly explains why image-only Vulkan has0 diagnostic batches
without missing image draws; it does not equate batches across backends, weaken
draw/pixel checks or claim optimization. This documentation increment adds0findings.

## Evidence boundary

Only this authorized report was written. No build, test, script/fixture,
GPU/native execution, process/session inspection or spawn. Parent-reported
dev/manual and pure4 passes are not independently rerun. Actual structure,
runner fixtures, release/native and full regressions remain root-owned.

**Standards:0 open findings; historical P2 closed; Spec axis not owned here.**
