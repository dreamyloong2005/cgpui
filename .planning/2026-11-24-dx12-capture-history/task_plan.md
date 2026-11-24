# Capture history compatibility

2026-11-24. Baseline `58e16f71`; overall DX12 goal active, local commit/no push.
Human explicitly chose Vulkan-compatible last-successful requested capture,
recorded in completion/public-contract-decision.md. Image batching is separate.

Built-in capture-capable renderers retain successful capture on the same valid
surface/device through uncaptured success, rejected input and abandonment.
Failed candidates never publish. Fresh/cold-without-success is null. A later
successful requested capture replaces the owned history. Borrowed pointers need
copying before subsequent operations; resize/recovery/reset remain explicit
invalidation boundaries, not guaranteed preservation across surface/epoch change.

Owner edits are narrowly scoped: remove frame's pre-Present pixels reset and
only move an engaged candidate into pixels at the existing success endpoint.
Keep debug/Commit/completion first and the nonthrowing final move. No recovery,
orphan, retained-image, fallback/Popup or image-batch policy changes.

External A/B rebased onto accepted orphan source:
`D:/Dev/shots/cgpui-dx12-public-contract-20261124/capture/rebased-orphan/`.
Independent original and rebase-preservation source reviews0; unexecuted.

1. Apply A shared exact history test/registration. Run explicit Vulkan and DX12.
   Expected semantic DX12 exit8 after real captured red then uncaptured green;
   setup/native failure is not this RED. No backend-conditioned expectation.
2. Apply B focused production + precise native late/early failure expectation,
   structure and public leaf/doc migrations; preserve all orphan owner/request
   changes. Same shared test GREEN; cold/warm native late InfoQueue test must
   inspect every error and preserve old successful pixels only in warm case.
3. Structure/native/shared regression and independent actual-source reviews;
   freeze full Windows/WSL/hardware/WARP/debug/GPU/ASan/SDK matrix with source
   and exact inventory checks before local commit. No claim of total completion.

## Observed progress

- Actual A compiled; identical test passes Vulkan/Intel and fails DX12/WARP
  exit8 after successful red capture followed by successful uncaptured green.
  Error is precisely lost last-success capture, not setup/presentation failure.
- Applied B after fresh check, preserving orphan branch. Exact shared Vulkan and
  DX12 tests now pass; native real late-error cold/warm both0, with cold null and
  deep prior-success pixels preserved only in warm case. Actual-source final
  independent Standards/Spec reviews0. Structure228/native136 WARP-debug passed;
  full frozen matrix next. No final acceptance claim yet.
- First frozen driver failed Windows workspace846/1: ordinary_menu_cancel_test
  exit4, opaque reference timed out before constructing a DX12 renderer. No
  NATIVE_LOOP_OPEN observation was logged. Original frozen logs are retained.
  Identical isolated default-adapter reproduction passed without source change;
  bounded5-repeat replay also passed all5 with unchanged source. Root cause not
  established; no timeout, oracle or skip policy weakened. Existing log cannot
  distinguish popup discovery/highlight/stability wait at the timeout. This remains
  a native-menu fixture reliability gap, not a claimed fixed capture defect.
  A fresh complete matrix runs in verified/ with original final/ failure retained.
- Second complete frozen matrix driver exit0: Windows847/WSL671, all builds,
  native136 per adapter, shared97 per selection, ASan136/97+97, fixtures and
  installed SDK pass. Exact inventories/no native-common skips and2998 source
  consistency checks pass. First-run timeout remains disclosed and unlocated.
