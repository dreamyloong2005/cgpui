# Acceptance: experimental DX12 bootstrap

Date: 2026-11-23 (Asia/Shanghai). Baseline `4d49b9ec` on `phase-v`.
This is the first executable DX12 milestone, not completion of the full DX12
design or production/default-backend certification. No push or publication.

## Delivered contracts

- Windows has an independent renderer dispatcher plus named Vulkan/DX12
  factories. Unset/empty/`auto` remains Vulkan; explicit backend failures do not
  silently select a different backend. Native backend/adapter information is
  available through Renderer and Window; unchanged Metal reports unknown.
- DX12 creates a real hardware or explicitly selected WARP device, direct queue,
  fence, root signature/PSO and opaque two-buffer HWND swapchain. Device selection
  checks feature level 11.0 and shader model 6.0. Debug configuration is fixed
  before the first DX12 device in a process.
- Clear, solid/rounded rectangles and selection/caret rectangles support DPI,
  clipping, composed transform/opacity and original draw order. CPU rectangle
  geometry belongs to neutral planning, with thin Vulkan compatibility forwarders.
- Pixel readback, resize, overlapping-frame rejection and frame-outliving-renderer
  handling are implemented and tested. The baseline waits for GPU completion
  per submitted frame; it does not implement an upload ring or throughput parity.
- Unsupported text, images, shadows and blur reject the frame explicitly.
  Transparent/composition surfaces are rejected; DX12 Popup capability is false.
  No new public native/composition window mode was introduced. Vulkan still
  attempts Popup and only falls back for missing capability or actual failure.
- Shared rectangle Slang emits embedded DXIL through the canonical shader tool;
  all five Vulkan SPIR-V instruction payloads remain byte-equivalent to baseline.
- Windows examples use the dispatcher package. Base packaging includes dispatcher
  and DX12 archives; SDK metadata describes embedded DXIL. CI guards the package
  graph on Windows/Linux/macOS. No native Windows dependencies enter Linux builds.

## Verification

Evidence root: `D:/Dev/shots/cgpui-dx12-bootstrap-20261123/`.

| Verification | Result | Evidence |
| --- | --- | --- |
| Windows complete suite, serial execution | **642 passed, 0 failed** | `windows-final-test.log` |
| Windows all libraries/examples | PASS | `windows-all-build.log` |
| Architecture checks (existing caps unchanged) | **197 passed** | `architecture-test.log`, full suite |
| DX12 WARP + Debug Layer | **6 passed** | `native-warp-debug-green.log` |
| DX12 Intel Graphics + Debug Layer | **6 passed** | `native-adapter-0-debug.log` |
| DX12 NVIDIA RTX 5070 Ti Laptop + Debug Layer | **6 passed** | `native-adapter-1-debug.log` |
| DX12 WARP + GPU-based validation | **6 passed** | `native-warp-gpu-validation.log` |
| Expanded hidden/visible nonactivating HWND test, all three adapters | PASS | `native-visible-{warp,0,1}.log` |
| External installed-only Windows headers/libraries, /MT, /OPT:NOREF, real WARP capture through dispatcher | PASS | `installed-sdk-windows.log`, `installed-sdk-windows/verification.json` |
| WSL complete suite (private Xvfb) | **609 passed, 0 failed** | `wsl-full-test.log` |
| WSL all libraries/examples | PASS | `wsl-all-build.log` |
| External installed-only Linux header/library link and consumer run | PASS | `installed-sdk-linux.log` |
| SDK headers / backend package graph Python fixtures | **15 + 4 passed** | `wsl-driver.log`, package closure logs |
| Scoped Windows AddressSanitizer DX12 suite, WARP + Debug Layer | **6 passed, 0 failed** | `asan-dx12-build.log`, `asan-dx12-test.log` |

GPU capture and successful visible-window submission are not assertions about
desktop-compositor screenshots. WSL verifies Linux regression, not native macOS.

## Reproduced defects and review

- Capture after resize originally failed with `DX12 capture mapping failed
  (0x80070057)`. Corrected the read range to exclude nonexistent final-row pitch
  padding; the same regression passed (`native-capture-{red,green}.log`).
- The first full Windows run was 641/642. The failing clip/composition closeout
  test still expected four type declarations at Vulkan-owned paths. Assertions
  now point to the neutral owners; behavior checks and size caps are unchanged.
  Focused GREEN was followed by the complete 642/642 rerun.
- Independent Spec review found missing dispatcher/DX12 archives in the base
  Windows package. A new graph fixture demonstrated RED/GREEN around the repair.
- Standards review found no new module-boundary/cap violation. Documentation
  review clarified exceptional synchronization, loader and execution limits.
- Once-per-process Debug Layer setup is preventive API hardening, not a claimed
  RED/GREEN fix: the previous multiple-renderer regression already passed.

## Explicit remaining gates

Text/glyphs, images, shadows, blur, transparent presentation/Popup, device-loss
recovery and performance parity remain unimplemented. Repeated queue Signal,
event-registration or wait failures while the device remains alive have not been
fault-injected. Safe resource retirement after repeated synchronization failure
requires a dedicated seam/review before production readiness; normal lifetime
tests do not certify arbitrary GPU failure safety.

The dispatcher retains a normal Vulkan loader dependency; explicit DX12 selection
is not Vulkan-loader-free startup. No delay-load or loader-absent startup was
validated. Installed-library checks are local acceptance, not `/MD` release SDK
certification. Scoped ASan does not replace full release validation.

DXIL-only generation/check and dxc inspection passed. The all-target shader check
reports Mac-generated output differences between recorded Slang `2026.1` and
local `2026.1-52-gc8ddf20bb`; Metal outputs were deliberately not regenerated.
No native macOS execution or package publication is claimed.

Product/test/build sources match the snapshot used by the validation mirror;
only README, backend documentation and planning closeout changed afterward.
The user's pre-existing `.vscode/` is preserved and excluded from the commit.
WSL reuses the agent-owned `/root/cgpui-logic-cleanup-20261123` mirror and leaves
the user's dirty `/root/cgpui-linux` checkout untouched. Builds ran sequentially.
