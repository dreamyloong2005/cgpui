# Accepted: DX12 unsupported backdrop tint fallback

2026-11-24. Baseline `32096285`. Evidence:
`D:/Dev/shots/cgpui-dx12-blur-fallback-20261124`.
This bounded fix is complete; the overall DX12 completion goal remains active.

## Implementation and direct evidence

The focused backdrop-frame owner now performs its existing device-scaled raw
validation before capability routing. If blur is unavailable it delegates the
original logical draw to RenderFrame's shared tint fallback. Rounded geometry
performs the single DPI conversion; supported blur and real native errors retain
their previous paths. No public API, backend default or Popup policy changed.

A-only compiled five tests, then real WARP pixel capture/present failed exit3
with unsupported error4 after native identity was established. This is recorded
in stage-a-pixels-red.log; no stub/fake presentation supplied that RED. Applying
B's single-file change made the same scene and all five focused tests pass.

The tests cover tint/no tint/alpha0, ordering and rounded cutouts; scales1/1.5/2,
metadata versus composition stack, clips and empty geometry; original invalid
input and next-frame behavior; supported→unsupported→supported target lifetime;
and real RemoveDevice during an outstanding frame followed by typed retry and
recovered captures. Diagnostics preserve actual rounded/no-op/pending-command
semantics and report no fallback offscreen work. Cap absence is injected into
private state after real initialization, not observed on an incapable GPU.

## Final frozen matrix

- Windows: **783 pass**, 0 fail; all-member build passes.
- Dedicated WSL mirror: **648 pass**, 0 fail; all-member build passes.
- Architecture: **213 pass** inside full suite; original caps unmodified.
- Intel/NVIDIA/WARP native DX12: **102 each**, debug enabled; WARP GPU validation.
- Common renderer DX12 Intel/NVIDIA: **92 each**. WARP explicit-both: Vulkan92,
  DX1292 with actual software identity; all13 shared pixel scenes remain intact.
- ASan DX12: **102 pass**; ASan renderer explicit-both: **92 + 92 pass**.
- Driver fixtures: PS7 **17 pass**; PS5 **15 pass + 2 PS7-only skips**.
- SDK graph/header fixtures: **9 / 15 pass**.
- Installed consumer actual /MT compile/link/run passes opaque/Popup/client-alpha
  captures and texture cold/warm/replacement checks with no Vulkan import/load,
  both clean and owned-invalid-loader cases. CMake configure-only passes.
- Windows raw and WSL normalized source checks both match all **2868 paths**,
  excluding only planning and user .vscode. Final script exits0.

No final-matrix retries or native SKIP markers; intentional initial RED is retained.
Independent Standards and Spec reviews each report0 actionable findings and
confirm no semantic drift from the reviewed candidate. Their source review is
not substituted for root's execution.

## Limits

Failed CheckFeatureSupport queries still map to support=false/reason, as before;
this fix does not distinguish missing capability bits from that probe error.
Later native failure/removal is not swallowed. Tests do not certify arbitrary
TDR, unavailable old OS, native macOS, all desktop/chrome/menu combinations or
performance. Installed archives are hardlinks, not an immutable release; CMake
was not a binary/CRT build. Default/auto remains Vulkan; explicit DX12 does not
fall back. No push/publication. Pacing, coordinated cross-thread recovery and
remaining compatibility/release gates remain pending.
