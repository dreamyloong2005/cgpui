# Actual-source Spec review — steady-state baseline

2026-11-24. Reviewer `/root/popup_structure`.
**Spec: 0 actionable source findings. Runtime acceptance remains open.**

## Scope and preservation

Baseline/HEAD: `d794f1145e339ed2c459d2bc69659e2dd236c0ae`.
Reviewed `git diff HEAD`, all25 untracked benchmark/structure/reproducibility
paths, and normative `task_plan.md:8-18,22-41`; `.vscode` was excluded.
Compared actual contents with reviewed `revised-creation` integration
`9a3570beabfd5bf4540fc970f18eae48205226fe18c94d01287dede230d43d98`
and four-file `reproducible-tools` supplement
`0954d03ac3f39907a8cf15bb82862ba4a6937d69066b3d0946d6bab90914b9af`.

After CRLF/LF normalization, **24/25 paths are identical**. The remaining
structure test differs by exactly the two section-anchor replacements below.
The three tracked diffs each add one workspace/discovery/CI registration;
existing missing-image/native-reuse entries are preserved. No production
renderer, public API, pacing or measurement-source change was introduced.

## Structure correction

`tests/architecture/renderer_steady_state_structure_test.cpp:81-82` now searches
`"\n[test]\n"` and `"\n[target."`. Previously the first `[test]` occurred in the
manifest's introductory comment, so the inspected substring incorrectly included
the manual executable declaration. The new anchors select the actual section
lines in this manifest. Missing-section rejection, manual-bin exclusion, all four
exactly-once pure-test assertions and all other guards remain unchanged. Reversing
only these replacements exactly reconstructs the reviewed candidate. Current
guard SHA256: `9ea7b769151f34d3dc674fe7e7342ff3ea3b08829444fb215c575ba4836d428d`.

## Measurement contract

Re-read actual sources: measured named-factory time excludes identity/fixture
work; preflight, renderer-cold, warmup, steady and final capture remain separate.
Only readiness acquisition retries; Present is once. Steady rows require exact
work, zero image/glyph uploads, no capture history and stable identity/display
epochs. CSV retains every accepted row; statistics use steady samples only with
nearest-rank percentiles. Native timed execution remains opt-in, outside discovery.

Actual runner/fixtures/documentation match the reviewed supplement: owned-process
timeout124 and evidence-IO failures remain failures; synthetic helper success is
not native evidence. Profile/source labels and NDEBUG do not certify optimized
build provenance. API waits are not GPU time/FPS/speedup.

## Acceptance limits

Root reports dev/manual compilation and four pure passes; those are not rerun
or independently certified here. Corrected structure/full suites, three runner
fixtures, release provenance, native positive/negative controls, repeated audited
three-scene outputs and full regression gates remain root-owned. This report
does not mark the normative acceptance sequence complete.

Only this review was written; no build/test/fixture/parser/native/GPU execution,
engineering edit, root-session inspection or agent spawn occurred.

## Addendum — LF/CRLF section-prefix correction

2026-11-24. The preceding guard hash describes the earlier snapshot. Standards
review identified a compatibility issue this initial Spec review missed:
`"\n[test]\n"` does not match an untranslated CRLF manifest on Linux because
`\r` follows `]`. That finding is retained, not retroactively represented as
passing cross-platform execution.

Read the current guard: line81 now uses `manifest.find("\n[test]")`; line82
remains `manifest.find("\n[target.", begin)`. Current raw SHA256:
`dd2d4fbd1f951bf3d9f2fca588c55e5c189e208211eebb3223bcdd57875a2a4f`.
Reversing only these two anchors still exactly reconstructs the external reviewed
candidate after newline normalization. No other guard statement changed.

The leading newline still excludes `[test]` embedded in the first-line comment;
omitting the trailing newline accepts both LF and CRLF after the real section
header. Missing-section rejection, the prohibition on the manual executable in
discovery, four exactly-once pure-test predicates, module caps and exact registry
assertions all remain. This corrects input line-ending recognition, not the
measurement contract or acceptance predicates. **0 additional actionable Spec
findings in the current increment.**

Root reports raw LF/CRLF positive and manual-injection rejection fixtures, a
successful release build and two dev native smoke runs with20 valid samples;
those are attributed updates, not this reviewer's executed evidence. The revised
C++ structure run, formal release provenance/comparisons and remaining acceptance
gates still require root verification. This addendum used source/text/hash
inspection only; no test, fixture, parser, build, GPU operation or spawn.

## Addendum — backend-specific diagnostic batch explanation

2026-11-24. The earlier statement that actual documentation matches the external
four-file supplement describes that earlier snapshot, not the current bytes.
Read the latest `docs/renderer-steady-state-baseline.md:79-84`; current SHA256:
`ba68a149c1392f76cc9f75afbe8d3fef98311e2dd391c9fda74b669db028ca50`.

The sole documentation increment in the observed comparison is the five-line
explanation that DX12 diagnostic batches count visible draw groups, while the
current Vulkan legacy planner excludes image commands. Thus a Vulkan image-only
scene can report zero batches while submitting its image draws. The text keeps
exact draw counts and pixel checks authoritative and forbids interpreting these
different batch conventions as native submission counts or optimization gains.
This matches the actual planner/submitted-work source chain independently reviewed
in the external `report-audit/revised-batches/review-independent.md`.

**0 actionable Spec findings in this documentation increment.** It changes no
measurement source, native rendering, sampling, acceptance predicate or previously
recorded run. This is not a claim that the two documentation files remain identical
or that any pending runtime gate passed. Only this addendum was written; no test,
fixture, parser, build, GPU operation or spawn was performed.
