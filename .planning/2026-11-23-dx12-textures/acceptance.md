# Acceptance: DX12 text and image milestone

Date: 2026-11-23 (Asia/Shanghai). Baseline `4e0455f0`, branch `phase-v`.
Continuation of the experimental opaque-window backend, not completion of the
full DX12 design or all of phase4 (which also includes shadows).

## Delivered behavior

- Native text reuses the existing shaper/rasterizer, persistent GlyphCache and R8
  atlas pages. Public tests verify actual DirectWrite A/B masks and whitespace,
  coverage, composition opacity, transforms, clip intersections and DPI scaling.
- RGBA8 images support nearest/linear clamp sampling, source crop and tint;
  padded caller rows are repacked before aligned GPU uploads. Invalid IDs,
  dimensions, byte ranges, formats, descriptors and nonfinite geometry produce
  errors without poisoning subsequent valid frames.
- A typed ordered command stream interleaves rectangles, rounded rectangles,
  text and images. Each draw holds an immutable texture snapshot: later
  replacement, invalidation or glyph-page growth cannot change an earlier draw.
- Text and image CPU vertex algorithms now live in focused neutral planning
  modules, with Vulkan aliases/thin forwarding and unchanged algorithms/caps.
  Six new public leaf headers are independently compiled and installed-header
  boundaries remain free of native implementation dependencies.
- Text/image PSOs, root signature, sampler selection and R8/RGBA8 texture
  uploads are focused private DX12 modules. Shared Slang generates embedded
  DXIL; all five Vulkan SPIR-V instruction payloads are unchanged from baseline.
- Images are evicted after more than120 successfully presented idle frames.
  Upload/invalidation updates CPU cache at command-recording time: rejected or
  abandoned frames do not roll back those operations. GPU realization/upload
  is lazy, and readiness is published only after fence completion.

Vulkan is still the default, including `auto`. Explicit DX12 failure does not
silently fall back. Transparent/Popup capability remains unavailable in DX12;
Vulkan's existing supported Popup/failure-only fallback policy is unchanged.

## Verification

Evidence root: `D:/Dev/shots/cgpui-dx12-textures-20261123/`.

| Verification | Result | Evidence |
| --- | --- | --- |
| Windows complete suite | **659 passed, 0 failed** | `windows-final-test.log` |
| Windows all libraries/examples | PASS | `windows-all-build.log` |
| Neutral planning suite | **9 passed** | `neutral-green.log` |
| Public header suite | **40 passed** | `headers-green.log` |
| Architecture, no relaxed existing caps | **198 passed** | `architecture-test.log` |
| Expanded native DX12, WARP + Debug Layer | **13 passed** | `native-expanded-test.log` |
| Final native DX12, Intel Graphics + Debug Layer | **14 passed** | `native-final-0.log` |
| Final native DX12, NVIDIA RTX5070Ti Laptop + Debug Layer | **14 passed** | `native-final-1.log` |
| Final native DX12, WARP + GPU-based validation | **14 passed** | `native-final-warp-gpu.log` |
| Additional invalid-image recovery regression, WARP + Debug Layer | **1 passed** | `image-validation-test.log` |
| Hello-window input/text/clipboard/redraw/close smoke with explicit DX12 + WARP | **1 passed** | `hello-dx12-smoke.log` |
| External installed-only Windows consumer, /MT + /OPT:NOREF, actual text/image capture | PASS | `installed-sdk-windows.log`, `installed-sdk-windows-v2/verification.json` |
| WSL complete suite | **618 passed, 0 failed** | `wsl-full-test.log` |
| WSL all libraries/examples and installed-only consumer | PASS | `wsl-all-build.log`, `installed-sdk-linux.log` |
| Scoped Windows ASan native DX12 suite, WARP + Debug Layer | **14 passed, 0 failed** | `asan-dx12-test.log` |
| SDK header/package graph Python fixtures | **15 + 4 passed** | local rerun, `wsl-driver.log` |
| DXIL-only canonical regeneration check / dxc inspection | PASS | `dxil-generation.log`, local `--dxil --check` |
| Five Vulkan SPIR-V payloads preserved | PASS | `spirv-preservation.json` |

The final14-test matrix includes the later invalid-image regression; the earlier
13-test checkpoints are retained. GPU readback/visible submission and automated
example lifecycle checks do not constitute desktop screenshot verification.

## Demonstrated feedback loops

- `image-red.log`: native image upload rejected, exit5; same public pixel test
  passes after implementing the texture/image path.
- `text-red.log`: native text rejected, exit13; same public text pixel test passes
  after implementing shared glyph planning and atlas texture submission.
- `neutral-red.log`: missing public neutral owners; new behavior/ownership tests
  subsequently pass. `shader-red.log`/`shader-green.log` cover embedded DXIL owners.
- `cache-eviction-red.log`: exit11 retained images beyond120 idle frames. Added
  successful-frame aging; the same regression passes in the expanded suite.
- Invalid-image test's initial compilation failed on the Windows `max` macro;
  parenthesizing its numeric_limits calls fixed the test. This is not reported
  as a product rendering defect.
- External installed-consumer helper initially used an ambiguous empty-brace
  font argument to shape_text; using explicit FontDescriptor fixed the helper.
  Verification then compiled/linked/ran in a fresh installed-only output folder;
  product sources did not change.

## Standards review

No outstanding finding. Existing source caps were not raised, public leaves
remain declaration-only, and native text/images consume the neutral algorithms.
Build/behavior verification is reported separately above.

## Spec review

No outstanding actionable deviation within this milestone. Immutable snapshots
and ordered commands preserve required rendering semantics; the added shaders
remain embedded in the existing DX12 library without new SDK dependencies.
Single-resource SRV heaps and synchronous uploads are an incremental strategy,
not implementation of the design's full pooled descriptors/upload ring.

Summary: Standards0 outstanding; Spec0 outstanding. Neither review certifies
the explicitly deferred production-readiness gates.

## Remaining limits

Shadows, blur, transparent presentation/Popup, device-loss recovery, upload-ring
and descriptor-pool optimization, diagnostics parity and automatic/default DX12
selection remain future work. Glyph pages are lifetime-cached and dirty pages
are uploaded as whole immutable snapshots, not partial dirty ranges. No bounded
glyph-cache or performance-parity claim is made.

The existing repeated queue Signal/event/wait failure retirement risk is not
solved or fault-injection-certified here. Normal and invalid-command lifetime
tests do not establish arbitrary GPU failure safety. The dispatcher still uses
the normal Vulkan loader dependency; no loader-absent startup was verified.

Metal outputs were not regenerated; no native macOS test or SDK release is
claimed. WSL validation uses only the agent-owned validation mirror, never the
user's dirty Linux checkout. `.vscode/` is preserved and excluded from commits.
No push/publication. Product source is frozen at the validation snapshot; only
dated evidence/planning closeout should change after verification.
