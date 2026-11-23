# Review: DX12 texture staging upload pages

Date: 2026-11-24. Fixed baseline `241b226a`, branch `phase-v`.
Reviewed working diff plus all new task source/test files; `.vscode/` excluded.
Spec: `task_plan.md`. Standards: root `AGENTS.md`, existing private module ownership
and established DX12 leaf caps, mcpp discovery and architecture CI registration.

## Standards — independent reviewer

Hard violations: **0**. Actionable Fowler-smell findings: **0**.
The common owner only manages allocation/mapping/retirement. Geometry and texture
retain independent instances and focused copy policies. GPU copy/barrier recording
stays in texture_resources. Public factory changes are comment-only. All nine
native tests and the new architecture guard are registered; private sources are
owned by the existing backend glob. Narrow lifecycle wrappers are intentional
safety boundaries, not unnecessary delegation. Reviewer did not build or run.

One nonblocking planning wording inconsistency was corrected: CPU footprint/row
preparation belongs in the texture adapter; native CopyTextureRegion and barrier
recording remain in the existing texture-resource recorder.

## Spec — independent reviewer, not an implementation author

Findings: **0**. No observed scope expansion.
The geometry contract (16B, UINT, stats) survives the extraction. Texture placement
is 512B; native row pitch and final-row bounds are checked; the CPU slice pointer
and resource-relative GPU footprint each receive their correct offset. Payload
accounting excludes padding. Rewind follows completion, old pools clear before
device retirement, unexecuted recording is not marked uploaded, and completed
uploads commit before inspecting Present's later failure result. Obsolete
state.uploads ownership is gone. Immutable and cached-snapshot policy is unchanged.
Reviewer inspected test oracles but did not execute tests.

## Root integration evidence and limits

- Real baseline RED passed 768 texels and 3072 diagnostic bytes, then failed the
  resource budget with 128 staging buffers (exit 5).
- Budget GREEN: one staging page; new-page counts 1/0/0 for cold/cache/replacement;
  texture payload counts 3072/0/3072. All 768 texels per frame pass.
- Nine targeted staging tests pass with WARP/debug, including actual GPU odd-row
  multipage/oversize copies, direct native-byte/allocator tests, immutable snapshots,
  recorded-but-unexecuted upload retry, synthetic completion-failure retention,
  and real idle-device removal/recovery. These are distinct validation seams.
- Full frozen-source matrix passed: Windows 770, WSL 645; native Intel/NVIDIA/WARP
  95 each; ASan DX12 95/renderer 89; SDK fixtures 9/15; installed consumer PASS.
  All 2825 source paths match frozen Windows and normalized WSL manifests. See acceptance.md.
- Native Create/Map error handling is source-reviewed; driver OOM or Map failure
  was not injected. Synthetic failure guards are not claims of arbitrary TDR
  recovery. This milestone is not whole-DX12 completion or FPS acceptance.
