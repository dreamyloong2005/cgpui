# DX12 unsupported-backdrop fallback

Date: 2026-11-24. Baseline: `32096285` (verified common-pixel/runtime/driver slice).
Status: IN PROGRESS; not full DX12 completion. Root owns all builds/tests/commits.

## Bounded change

- Keep existing device-scaled raw bounds/tint/clip validation before capability
  routing in `dx12_backdrop_blur_frame.cpp`; reuse the public base tint fallback
  with original logical `backdrop`, not its scaled copy. Supported planning and
  native errors retain their existing paths. No new API, fault environment flag,
  default switch or broad-file implementation.
- Five real native tests cover pixels/order, scales/composition/clips, validation
  and next-frame recovery, supported/unsupported target lifetime, and actual
  RemoveDevice interrupting an unsubmitted frame. New focused support and structure
  guard preserve original caps and current common/runtime registrations.
- Capability absence is forced only through private state after real native
  initialization; this is not evidence from an incapable physical adapter.

## Sequence / evidence

- [done] External A/B prepared; independent candidate review reported no findings.
- [done] Root applied A's 12 test/support/registration files to the repository.
- [done] Root compiled all five native tests and observed the first pixels-test
  native RED: exit 3, actual DX12 Microsoft Basic Render Driver, software=true,
  scale=1, actual blur support=true; forced unsupported reached present error 4.
  Evidence: `stage-a-build.log` / `stage-a-pixels-red.log`.
- [done] Root applied B after that meaningful RED and recompiled all five tests.
  The actual source now validates before delegating the original logical draw.
- [done] Root reran all five native fallback tests under WARP + DX12 debug:
  **5/5 PASS**, including scale 1/1.5/2, in `stage-b-test.log`.
- [pending] Broader supported blur/diagnostic/recovery regression,
  structure/header/common backend checks and root-selected final full matrix.
- [done] Updated `docs/dx12-backend.md` after B was actually applied; describe
  tint-only/no-tint behavior, strict validation, diagnostic semantics and evidence
  limits. Replaced the inaccurate blanket unsupported-capabilities rejection claim.
- [pending] Final source/evidence review and local commit. No push or release.

The previous slice is committed and separately verified in its own evidence
record. Its historical counts are not reused as acceptance for this new change;
this slice's final full matrix is still pending.

## Acceptance boundaries

No tint adds no work. Visible tint (even alpha/opacity zero) remains one rounded
command/draw; empty or fully clipped geometry keeps one planned and zero submitted
commands, so its one pending command is not an exact-work failure to conceal.
Fallback blur primitives/effects/offscreen passes/targets remain zero; shared
rounded geometry controls real tint pixels. Do not weaken malformed-input checks,
hide genuine API failures, turn device-loss retry into successful present, or
change sigma/metadata normalization as a side effect.

The existing capability probe represents failed CheckFeatureSupport HRESULT and
missing format bits with the same support=false/reason result. This slice neither
claims to distinguish them nor changes that public type. See the scoped design
in `../2026-11-24-dx12-completion/blur-fallback-next.md` and external candidate
README for the executable RED/GREEN procedure; no build/test was run by the docs
owner and no unreported acceptance is recorded here.

## Root intermediate frozen validation

Original and new backdrop structure guards passed 2/2. Standards and Spec both
reviewed the applied A/B payload against the external candidate (no semantic
drift) and current docs with 0 actionable findings. Source frozen at 2868 paths.
The full sequential script has so far passed Windows783, WSL648, all-member
builds, Intel/NVIDIA/WARP native DX12 suites102 each, common Intel/NVIDIA DX12
92 each and WARP explicit-both92+92. ASan/fixtures/installed SDK/final source
checks are still running; this is not final acceptance yet.

## Final closeout

All final gates passed, script exit0, 2868 frozen paths consistent on both hosts.
See acceptance.md for exact counts and validation limits. This bounded slice is
complete; the overall DX12 goal remains active. Next prepared prerequisites are
Win32 delayed-timer failure handling and neutral runtime readiness scheduling,
followed by actual DXGI pacing activation. External candidates are not accepted
production until their own RED/GREEN and regression gates run.
