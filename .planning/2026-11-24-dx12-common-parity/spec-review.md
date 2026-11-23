# Independent Spec review

Date: 2026-11-24. Reviewer: `/root/common_spec_review`.
Baseline: `36b3cd40`; scope: working-tree diff plus new common/runtime/driver
files, excluding user `.vscode/`. No commits exist after the baseline at review.

## Result

**0 confirmed actionable findings.** No P1/P2 spec defect found in this bounded
slice; this is not certification that the complete DX12 goal is finished.

- Compared all 13 old pixel sources against their new shared counterparts.
  Original pixel samples, tolerances, shadow math, text silhouettes and ink
  extents remain intact. The common creator makes the selected named factory
  call and rejects actual backend/selection/fallback mismatches, including a
  nonsoftware result for WARP. Default/auto remain named Vulkan in this harness.
- The moved Vulkan 0/5/9 and new DX12 0/3/6 backdrop assertions stay exact in
  independent named-backend wrappers; the one-effect two-frame case, allocation
  checks and submitted counts remain covered rather than weakened to parity.
- `both` runs two independent invocations with the same forwarded arguments.
  Ordinary native failures retain the first nonzero code without skipping the
  second run; genuine setup/invocation exceptions remain fail-fast. ASan setup
  and member translation are outside the loop. The final preference lookup is
  safe when absent under PS5 StrictMode; environment/location restoration and
  the genuine native `.cmd` exit boundary have focused fixtures.
- Both runtime variants use native platform/run_app creation and native DX12
  frames. The observer only requests capture and forwards the same frame/error.
  Real RemoveDevice, typed retry counts, fresh generation/epoch, per-window
  once-uploaded image/glyph payloads and recovered warm-cache controls cannot
  pass by substituting a successful frame or test-side retry loop.
- Windows-only dependencies and registrations preserve Linux/Metal boundaries.
  Final docs distinguish shared scenes from Vulkan-private tests, local bounded
  results from release readiness, and fixture CI registration from hosted GPU
  acceptance. The condensed getting-started guide retains the existing cap.

## Verification limit

Read-only source/spec/fixture review; no builds, tests, GPU runs or mutation
experiments were performed by this reviewer. Root owns final frozen-source
Windows/WSL/hardware/WARP/ASan/SDK execution. Full ordinary-window compatibility,
cross-thread recovery, pacing and remaining completion gates are not waived.
