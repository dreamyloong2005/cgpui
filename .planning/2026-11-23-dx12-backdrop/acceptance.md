# Acceptance: DX12 backdrop blur

Date: 2026-11-23 (Asia/Shanghai). Baseline `f81fa683`, branch `phase-v`.
This is the next opaque-window milestone, not complete DX12 parity.

## Delivered behavior

- Native backdrop commands snapshot previously recorded scene content and run
  the shared downsample, horizontal/vertical Gaussian and rounded composite
  shaders. Tint, composed opacity, clips and DPI follow the existing public
  contract. Later primitives remain on top; later effects see earlier effects.
- CPU planning now lives in four native-free public leaves and focused neutral
  implementations. Vulkan retains a thin type adapter and its own format
  capability/renderpass-count policy. Normal-size float planning is unchanged.
- Extreme finite sigma no longer overflows its CPU square or integer radius.
  Unrepresentable extents above INT32_MAX return an empty plan as documented;
  large representable extents use safe boundary arithmetic. Actual native
  renderer extent limits are unchanged. NaN/infinite/negative sigma still
  normalizes to zero, preserving the existing policy.
- The native format probe checks actual RGBA8 rendering/load/filter-sampling
  support. PSOs/root are lazy. Every visible effect owns an immutable scene
  snapshot, two region targets and descriptors until successful next-frame idle
  or destruction. No-effect frames allocate no offscreen GPU resources.
- Native owners separate support, validation, frame preparation, root, PSOs,
  targets, snapshot recording, filter passes and composite. Existing module
  caps were not raised. The unused unsupported-primitive helper was removed.
- Existing five Vulkan SPIR-V and four DXIL instruction payloads are preserved.
  New backdrop DXIL comes from the shared Slang source, with explicit DX12-only
  Texture2D/SamplerState binding to avoid an invalid combined-sampler Load
  lowering. No copied Gaussian algorithm or runtime shader compiler was added.

Vulkan remains default (including auto); explicit DX12 does not silently fall
back. Vulkan Popup/failure-only fallback and DX12 transparent/Popup rejection
remain unchanged. No public native/composition window mode was introduced.

## Verification

Evidence root: `D:/Dev/shots/cgpui-dx12-backdrop-20261123/`.
The serial Windows/WSL/installed-consumer/ASan driver completed with exit0.
The frozen archive/manifest contains2664 files, excluding build output,
dated planning and pre-existing user `.vscode/`. Final source-match.json confirms
no new, removed or changed source files after validation; only planning closeout
notes changed. This completes this milestone, not the deferred parity gates.

| Verification | Result | Evidence |
| --- | --- | --- |
| Frozen Windows complete suite | **685 passed, 0 failed** | `windows-final-test.log` |
| Windows all libraries/examples | PASS | `windows-all-build.log` |
| Windows installed-only SDK compile/link/run, including actual blur pixels | PASS | `installed-sdk-windows.log` |
| Frozen Linux/WSL complete suite | **633 passed, 0 failed** | `wsl-full-test.log` |
| Linux/WSL all libraries/examples | PASS | `wsl-all-build.log` |
| Linux installed-only SDK compile/link/run | PASS | `installed-sdk-linux.log` |
| Scoped Windows DX12 ASan + WARP + Debug | **25 passed, 0 failed** | `asan-dx12-test.log` |
| Final native Intel Graphics + Debug | **25 passed** | `native-0.log` |
| Final native NVIDIA RTX5070Ti Laptop + Debug | **25 passed** | `native-1.log` |
| Final native WARP + Debug + GPU-based validation | **25 passed** | `native-warp.log` |
| Native first WARP + Debug suite | **20 passed** | `native-first-suite.log` |
| Backdrop effects/mixed/lifetime/validation checkpoints | **5 passed** | `native-backdrop-lifecycle-test.log` |
| Affine nonuniform-scene + DPI sigma checkpoint | **1 passed** | `native-transform-test.log` |
| Neutral planning, including extreme sigma/extent | **15 passed** | `neutral-final-green.log` |
| Public header suite, including four new independent leaves | **47 passed** | `windows-final-test.log`; `headers-green.log` |
| Architecture with unchanged caps | **200 passed** | `architecture-test.log` |
| SDK graph/header Python fixtures | **4 + 15 passed** | `sdk-{graph,headers}-fixtures.log` |
| Canonical DXIL regeneration and dxc validation | PASS | `dxil-generation.log`, `dxil-check.log` |
| Existing SPIR-V/DXIL payload preservation | PASS | `shader-payload-preservation.json` |
| Actual backdrop demo first frame, hidden + explicit WM_PAINT | **exit0** | `backdrop-demo-smoke.log` |
| Workbench first frame, hidden + explicit WM_PAINT | **exit0** | `workbench-smoke.log` |

Public native acceptance uses actual GPU readback, not mocked renderers. Gaussian
step references use independent std::erf values; tint/composition tests use
literal colors. Native glyph coverage is used only as the glyph-mask reference
for independently calculated pre/post-effect blending. Four sigma ranges,
rounded masks, clip intersections, cumulative opacity and DPI1/1.5/2 are covered.
Rotation, nonuniform scale and shear use a nonuniform screen-space scene and
independent Gaussian values, including DPI-scaled sigma and points outside the
transformed shape but inside its bounding box.

The mixed sequence interleaves two effects with rectangle/image/native-text/shadow
commands. Lifecycle checks cover abandon, overlapping-frame/resize rejection,
odd-size resize, several differently sized effect targets, subsequent no-effect
frames, multiple renderers and orphan frames. Input rejection is followed by
valid frames; extreme and normalized sigma cases have independent pixel oracles.

## Demonstrated feedback loops

- `backdrop-red.log`: first public native tracer failed at present with
  unsupported backdrop, exit5; the same tracer passes after integration.
- `neutral-red.log`, `native-structure-red.log`, `shader-red.log`: missing
  planner/header/native/generated shader owners failed before implementation.
- `dxil-combined-sampler-red.log`: canonical generation failed because Slang
  emitted Texture.Load with an invalid sampler argument. Explicit DX12 texture
  and sampler bindings fixed this without changing Vulkan or filter mathematics.
- `neutral-extreme-red.log`: large finite sigma produced infinity and
  radius=-2147483648; overflow-safe square and pre-conversion radius clamp pass.
- `neutral-extent-red.log`: overlarge extents were accepted, odd boundaries were
  rounded and integer conversions/ceil division were unsafe; the documented
  representability guard and wide boundary path pass the same regression.

The demo uses hidden startup plus an explicit paint message to its exact spawned
CGPUIWindow, as established by the preceding milestone. It runs the real
text/shadow/backdrop scene, but is only a first-frame lifecycle smoke, not desktop
screenshot, interactive menu or benchmark verification.
Both smokes were repeated against the final rebuilt binaries. Workbench's
default first frame does not itself prove interactive menu blur support.

## Standards review

Independent static review found0 actionable hard violations or smell findings.
Public/private ownership is focused; existing caps are unchanged. Native tests
remain focused public behavior tests with no new implementation coupling beyond
the explicitly mandatory module-ownership guards.

## Spec review

Static review found no confirmed scope deviation. A separate independent native
review checked resource states, descriptor retention, state restoration, snapshot
format, unsubmitted failure handling and scale/clip semantics without findings.
Final Spec closure reviewed all six native tests and their scope without
actionable findings. Standards0 outstanding; Spec0 outstanding.

## Remaining limits

Transparent windows/Popup, device-loss recovery, complete renderer diagnostics,
pooled targets/uploads/descriptors, performance parity and default-switch/release
verification remain future work. Current per-effect snapshot allocation is
correctness-first and can retain peak frame memory across resize until the next
successful frame-start idle or renderer destruction. It is not a pooling or
bounded peak-memory guarantee.

GPU completion remains synchronous. Repeated Signal/event/wait failure retirement
and device-loss safety remain explicitly unverified; these normal-path tests do
not resolve that production gate. The dispatcher keeps its normal Vulkan-loader
dependency. Metal outputs were not regenerated and native macOS was not tested.

WSL verification uses the agent-owned mirror only, never the user's dirty Linux
checkout. Pre-existing `.vscode/` is preserved/excluded; no push or publication.
