# Independent review

Baseline: `9d80e51d8ebc247f109c6ada619a99f2d4204345` on `phase-v`.
Scope: working diff plus new diagnostic source/tests, before the local commit.
Spec: `task_plan.md`; standards: root `AGENTS.md` and the code-review skill's
module/locality and code-smell baseline. Reviewers did not run builds.

## Standards

Reviewer: `repo_progress/diagnostic_semantics` (did not author production).

The private diagnostic owner, planning leaf and narrow native hooks preserve the
repository's module-first layout; public API remains unchanged. No remaining
actionable standards finding was reported after corrections. The earlier
premature clean review was explicitly withdrawn: root review caught a blur
pass/backbuffer index collision, a planned-membership filter that hid unexpected
actual work, and per-draw global linear searches creating quadratic overhead.
These were corrected before the green/full validation. A private ledger test
now proves unmatched observations are not filtered and verifies resource identity,
upload saturation and candidate lifetime.

The first full run additionally caught the missing CI architecture-header matrix
entry and a stale exact text-append signature guard. Both were fixed without
weakening module checks; the text guard now verifies original command and run
metadata. The Standards reviewer performed a read-only follow-up and reported
zero remaining actionable findings (matrix 255 entries/255 matching source files).

## Spec

Reviewer: `popup_structure` (did not author production).

No actionable mismatch or scope expansion found. Original command/category
identity, independently observed submissions, lazy unique texture upload bytes,
blur composite/offscreen accounting, success-only publication and preservation
through resize/device recovery match the locked contract. Public tests cover
all eight kinds, empty/scissor-cull versus pixel coverage, caches and split text
runs, failures/abandonment, real device removal and borrowed peer snapshots.
Root extended existing recovery tests for actual image/glyph reupload accounting
and snapshot retention after an interrupted present.

The text test oracle initially compared canonical raster-cache keys to original
paint keys. A targeted probe proved lookup failure before any pixel comparison;
matching the actual page/allocation bounds fixes the oracle without weakening
the native pixel checks. The zero-area image case likewise retains actual native
behavior: emitted degenerate geometry and its upload are counted, not suppressed
to satisfy a pixel-coverage assumption. A dedicated assertion preserves this
distinction. All temporary diagnostic probes were removed.

Summary: Standards 0 remaining actionable findings; Spec 0. Full runtime and
packaging evidence is recorded separately in `acceptance.md`, not inferred from
these read-only reviews.
