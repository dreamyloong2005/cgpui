# Retained immutable image payload

2026-11-24. Baseline `140cde6b`; overall DX12 goal active, local commits only.
Root alone owns builds/tests/GPU. Preserve user .vscode and other checkouts.

Scope: existing planned retained_image_byte_count in the neutral data-only
diagnostic leaf, optional unavailable vs known zero, independent saturation.
DX12 measures each live immutable RGBA CPU snapshot once by weak ownership
identity across cache/frames/submission, using tight bytes, excluding glyphs,
padding, caller storage and native/GPU/staging memory. Not RSS or performance.

Focused private tracker belongs to image cache. Registration after insertion;
its own allocation failure latches unavailable rather than publishing a partial
known sum. Candidate diagnostic preparation precedes exactly-once cache aging,
then allocation-free sample and nonthrowing history move in a focused completion
owner. Existing owned capture publishes afterward. No changes to image operation
semantics, Vulkan capture history, default, Popup or recovery selection.

External reviewed staged source:
`D:/Dev/shots/cgpui-dx12-image-retention-20261124/candidates/`.
Rebased A/B preserve the accepted capture fix and its structural ordering guard.
Independent pre-application Standards/Spec reviews0; not runtime acceptance.

1. Apply only Stage A (public fields + real Renderer regression/support + neutral
   schema test/registration). Build then run WARP/debug upload-only frame.
   Semantic RED requires real successful blue capture and existing diagnostics,
   then absent count, no image upload, and testcase exit40. Setup/compile failure
   is not RED.
2. Apply B focused implementation and exact completion/capture guard integration.
   Same native test must pass known16, first-copy16/warm0 and glyph exclusion.
3. CPU weak ledger/cache/completion/equality, retained finished frames, eviction,
   history and actual idle removal/reupload regressions. Re-run capture regression
   and all structure guards; do not loosen existing caps.
4. Independent actual-source Standards/Spec final review. Freeze source, full
   Windows/WSL builds/tests, hardware/WARP/debug/GPU validation, ASan, installed
   SDK and exact source/inventory checks before acceptance/local commit.

OOM fallback is source-reviewed unless a real focused fault fixture is run;
synthetic overflow is not a huge allocation, RemoveDevice is not hardware TDR.
Public fields extend SDK data layout; installed consumers must be rebuilt.

## Observed progress

- Stage A compiled successfully. Real WARP/debug upload-only Present and blue
  readback succeeded; diagnostics printed reported0/payload0/saturated0/uploads0,
  then testcase exit40 FAIL_UNREPORTED_RETAINED_PAYLOAD. This is the intended
  runtime RED, not a build/setup failure. Logs: red-build.log/red-test.log.
- Rebased Stage B applied after fresh check. The same targeted GREEN passed:
  reported1/payload16/saturated0/uploads0, then actual four-color first-copy16 /
  warm-copy0 and glyph-exclusion assertions. Logs: green-build.log/green-test.log.
- Full structure227/227 and native131/131 WARP-debug regression passed, including
  accepted capture safety and retained-image CPU/native/history/recovery cases.
  Actual-source final independent Standards/Spec reviews0. Neutral schema passes.
- Frozen2986path complete driver passed first run: Windows839/WSL670, both
  builds, native131 per adapter, shared95 per selected backend, ASan131/95+95,
  installed SDK fixed4/3076 retained payload and loader checks. Exact inventories
  and raw/normalized source matching passed. Slice accepted, whole goal active.
