# Acceptance: DX12 outer box shadows

Date: 2026-11-23 (Asia/Shanghai). Baseline `e4d551a2`, branch `phase-v`.
Continuation of the experimental opaque-window backend; not full DX12 parity.

## Delivered behavior

- Outer shadows reuse the original Vulkan CPU algorithm through neutral
  planning leaves, and the existing analytic Slang shader through embedded DXIL.
  CSS blur/sigma, spread, rounded radii, original-box cutout, offset, transforms,
  composed opacity, clipping and DPI keep their previous semantics.
- Shadows participate in the original rectangle/text/image command sequence.
  The lazy shadow PSO restores the shared extent root signature/constants;
  mixed draws retain their ordering and native state transitions.
- Focused native frame, validation, PSO and recording owners do not contain
  copied shadow algorithms. Rectangles and shadows share a private non-template
  indexed upload/draw owner. All primitive PSOs share their fixed-state defaults.
- Three new public leaves compile independently. Vulkan keeps thin aliases and
  forwarding; its five SPIR-V instruction payloads are unchanged. Metal output
  was not regenerated. Existing structure/line-count caps were not increased.
- Invalid generated geometry rejects a frame and subsequent valid frames still
  work. Nonfinite blur/spread and transforms retain existing neutral
  normalization rather than introducing different raw-input semantics.

Vulkan remains the default, including `auto`; explicit DX12 errors never silently
select another backend. Vulkan Popup/failure-only fallback is unchanged. DX12
transparent/Popup capability remains unsupported; no public window-mode API was
added. No cache policy, device recovery or synchronization policy was changed.

## Verification

Evidence root: `D:/Dev/shots/cgpui-dx12-shadows-20261123/`.
Frozen input: `source-snapshot.tar` and `source-snapshot.json` (2631 files;
excludes build output, dated planning and pre-existing user `.vscode/`).

Sequential Windows/WSL/installed-consumer/ASan validation completed with exit0.
The final source-hash check matches all2631 frozen files (`source-match.json`);
only dated planning/acceptance notes changed after the snapshot.

| Verification | Result | Evidence |
| --- | --- | --- |
| Windows complete suite, frozen sources | **670 passed, 0 failed** | `windows-final-test.log` |
| Windows all libraries/examples | PASS | `windows-all-build.log` |
| Windows installed-only SDK compile/link/run, real text/image/shadow pixels | PASS | `installed-sdk-windows.log` |
| Linux/WSL full suite | **624 passed, 0 failed** | `wsl-full-test.log` |
| Linux/WSL all libraries/examples | PASS | `wsl-all-build.log` |
| Linux installed-only SDK compile/link/run | PASS | `installed-sdk-linux.log` |
| Windows scoped DX12 ASan + WARP + Debug | **19 passed, 0 failed** | `asan-dx12-test.log` |
| Native Intel Graphics + Debug, after indexed extraction | **19 passed** | `indexed-native-0.log` |
| Native NVIDIA RTX5070Ti Laptop + Debug, after extraction | **19 passed** | `indexed-native-1.log` |
| Native WARP + Debug + GPU-based validation, after extraction | **19 passed** | `indexed-native-warp.log` |
| Neutral planning suite | **11 passed** | `neutral-green.log` |
| Public header suite, including three new independent leaves | **43 passed** | `windows-final-test.log`; `headers-green.log` |
| Architecture with original caps, including indexed extraction | **199 passed** | `windows-final-test.log` |
| Indexed ownership RED then GREEN | **1 passed** | `indexed-structure-{red,green}.log` |
| SDK graph/header Python fixtures | **4 + 15 passed** | `sdk-{graph,headers}-fixtures.log` |
| Canonical DXIL regeneration/validator check | PASS | `dxil-generation.log`, `dxil-check.log` |
| Five Vulkan SPIR-V instruction payloads preserved | PASS | `spirv-preservation.json` |
| Hidden workbench first frame, final binary, explicitly triggered WM_PAINT | **exit0** | `workbench-painted-smoke.log` (qualified below) |

The native tests include literal hard-shadow pixels and box cutout, independent
std::erf Gaussian falloff, positive/negative spread, rounded corners, affine
rotation/nonuniform scale/shear, nested clipping/opacity and DPI1/1.5/2.
Rounded soft shadows use independent 2D midpoint Gaussian area integration;
the reference is convergence-checked before comparison, not copied from the
shader approximation. The mixed sequence is image -> shadow -> text -> shadow
-> rectangle -> image.

## Feedback loops and harness qualifications

- `shadow-red.log`: public native shadow pixels failed with unsupported shadow
  drawing before implementation; the same test subsequently passes.
- `neutral-red.log`, `native-structure-red.log`, `shader-red.log`: missing
  neutral/ownership/shader seams failed before their corresponding owners.
- `neutral-radius-oracle-red.log`: the new test incorrectly demanded exact15
  from normalized float arithmetic yielding15.0000009537. Only the test changed
  to its existing1e-4 tolerance; no product geometry was changed for this failure.
- `indexed-structure-red.log`: the shared indexed owner was required before
  extraction and then passed. Its original upload retention and draw semantics
  were preserved; texture recording remains separate.
- An initial workbench command used a wrong executable path and never launched
  the program. The corrected `Start-Process -WindowStyle Hidden` run timed out
  with empty output. An explicit WM_PAINT posted only to that test process's
  invisible CGPUIWindow then completed first-frame shutdown, exit0. This isolates
  hidden-window scheduling in the smoke harness, not a shader/product fix.
  A forced hidden first frame is not visible desktop or full-interaction proof,
  and the default workbench first frame does not itself exercise menu shadows.

## Standards review

One nonblocking repeated indexed-recording suggestion was addressed by the
focused private helper and ownership regression. Independent static re-review
found no outstanding Standards finding; module caps were not raised.

## Spec review

No actionable deviation within this milestone. Shared geometry/shaders, command
order and existing normalization semantics were retained. Backend selection,
Popup policy, caches and synchronization were not silently expanded.

Summary: Standards0 outstanding; Spec0 outstanding. Neither review certifies
the deferred production-readiness gates.

## Remaining limits

Backdrop blur, transparent ordinary-window/Popup presentation, device-loss
recovery, pooled uploads/descriptors, diagnostics/performance parity and default
DX12 selection remain future work. The backend still awaits GPU completion each
frame; this milestone makes no throughput or performance-parity claim.

Repeated queue Signal, event-registration or wait failures have not been
fault-injected. Safe retirement after those exceptional synchronization failures
remains unresolved; normal/invalid-command tests are not device-loss evidence.
The Windows dispatcher retains its normal Vulkan loader dependency.

No native macOS validation or SDK release is claimed. WSL uses only the
agent-owned validation mirror, not the user's dirty Linux checkout. User
`.vscode/` is preserved and excluded. No push/publication.
