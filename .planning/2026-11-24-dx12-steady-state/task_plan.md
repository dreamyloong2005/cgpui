# Native steady-state baseline

Pending integration after accepted missing-image source. Overall DX12 goal active.
No claim of completed optimization or performance baseline before actual execution.

## Ownership

Use the independently reviewed revised-creation candidate only:
D:/Dev/shots/cgpui-dx12-performance-20261124/candidates/revised-creation/integration.patch
SHA256 9a3570beabfd5bf4540fc970f18eae48205226fe18c94d01287dede230d43d98.
It adds a dedicated tests/renderer-steady-state manual bin, focused private support,
four pure discovered tests and an architecture guard/exact CI registration.
No production API or pacing policy changes. No timed native bin in default tests.

Checkout reproducibility supplement lives externally in candidates/reproducible-tools:
unchanged runner/failure suffix and helper, adjusted fixture path/comments, new
usage/interpretation document. Independent source review pending at this snapshot.
Formal comparisons require actual release flags/binary provenance, not a label.

## Acceptance sequence

1. Finish and audit the frozen missing-image matrix; commit that slice first.
2. Recheck current patch applicability, preserve all missing-image/native-reuse
   registry entries, apply full revised-creation once (not incremental also).
3. Add reviewed reproducibility supplement. Compile pure/manual dev, run four
   pure tests and structure suite, run three bounded non-GPU runner fixtures.
4. Build verified release manual bin. Record compiler/build graph/source/driver/
   OS/power metadata. Run native success and wrong-identity/visibility/window-change
   controls serially, preserving each output and nonzero partial failure.
5. Three scenes, repeat complete controlled same-adapter/profile runs. Actual
   backend/LUID/software identity and all raw CSV/JSON rows must be audited;
   no midrun capture, texture uploads or dropped samples. Keep WARP/debug cohorts
   separate. Preflight and final exact pixels are outside measurements.
6. Actual-source independent Standards/Spec reviews; full unchanged-source
   Windows/WSL/adapter/ASan/SDK/inventory regressions. Commit locally, no push.

Creation is factory CPU wall time, cold is renderer-cold after preflight, steady
rows include API waits. DX12 synchronous completion and Vulkan deferred retirement
are different envelopes, not GPU timing/FPS. Display observations are bounded,
not all-driver/VRR certification. Valid measurements do not imply a speedup or
full DX12/default/release acceptance. SRV pooling follows measured baseline.

## Actual integration progress

Baseline d794f114. Applied revised-creation full patch once and reproducible-tools
4file supplement. Dev manual build and pure4 pass. Original architecture guard
misread manifest header comment [test] and failed; fixed to real newline-anchored
section, full232 passed. Standards then caught cross-platform CRLF trailing anchor;
removed trailing newline requirement, preserved leading newline. Actual LF/CRLF
raw section fixtures both pass/manual injection rejected; final focused compiled
guard passes. Independent actual-source Standards0/Spec0 with historical finding.

Runner non-GPU3 fixtures actually pass (success0, child0→evidence IO1, realtimeout124).
Auditor original37 synthetic and8 extra strict JSON/CSV/manifest-integrity fixtures
pass; these are not native evidence. Negative-controller3 IO/real195s timeout
fixtures pass, retaining rows/timeout124 and stderr metadata. Revised controller
was source-reviewed after an original case-retention/timeout P2; original preserved.

Dev native mixed smoke20 each Vulkan/DX12 pass. Release manual build succeeds;
all368 effective compile commands independently checked /O2 /DNDEBUG /MT, no /Od
or sanitizer. Binary hash9745a3bb54239698c51901d484f8e9a19286a89fc58fd6d219f8def05211a6e5;
actual/release-provenance records source/diff/graph/commands/driver metadata.
Release DX12 mixed smoke20 passes independent raw audit. Native wrong-LUID,
hide/move controls both backends pass at correct preflight/warmup phases. Separate
3000ms controls target measured-loop; require actual reported phase, not time guess.

Planned fixed18 release series (3 repeats×3scenes×2backends), same IntelLUID73219,
120warmup/600steady each. First three complete runs pass. Fourth Vulkan image-only
exits0 with exactpixels/draws/uploads but original auditor rejects batches0. Actual
Vulkan batch planner excludes images; DX12 counts draw groups. This is a wrong
auditor lower-bound assumption, not diagnosed missing native work. Stopped/preserved
four original runs; author preparing independently reviewed source-backed exact
backend/scene batch audit + fixtures before continuing. No production measurement,
pixel/draw/upload oracle or pacing changes to obtain a baseline.

External actual evidence root: D:/Dev/shots/cgpui-dx12-performance-20261124/actual.
Full regression/WSL/ASan/SDK and valid completed release series remain pending;
no performance gain, default switch or overall completion claim.

- Revised source-backed exactbatch auditor independently reviewed0; actual54
  synthetic+8 strict/parser/manifest controls pass. Four preserved original native
  runs re-audited without rerunning/overwriting; remaining14 follow the original
  plan. All18 release series pass raw audit,10800 steady rows plus18cold/2160warmup,
  per-series stable60Hz display/Intel73219/same verified optimized binary. Baseline
  paused after4 for auditor correction; timestamps retained, not continuous-session
  or GPU-time/FPS evidence. No outlier discard or invented performance threshold.
- Real negative controls additional3000ms attempts observe measured_no_capture
  hide and move rejection on both backends; initial750ms controls correctly remain
  warmup evidence, not relabeled. Wrong-LUID preflight failures remain separate.
- Additional NVIDIA/WARP release smoke20 each pass as separate cohorts, not part
  of the same-adapter18series comparisons. Root source freeze/full regression next.
