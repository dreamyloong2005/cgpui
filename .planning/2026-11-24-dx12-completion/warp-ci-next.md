# Next bounded CI scope: WARP, debug layer, and GPU validation

Date: 2026-11-24. **Planning only; no workflow registered or dispatched.**
Owning source inspected at HEAD `36b3cd40f8a7a19192902dab63ad26ad235c94f0`
plus the current working tree. Source is frozen for another acceptance pass.
This investigation ran no build, test, GPU probe, network installation, or remote
Actions operation. Only this planning note is written by the investigation.

## Recommendation: two explicit acceptance surfaces, not one green umbrella

1. **Hosted common-pixel WARP lane:** propose `runs-on: windows-2025`, not the
   moving `windows-latest` alias. Execute the exact common thirteen scenes plus
   their real-backend identity test, with WARP + debug + GPU validation. These
   use a real Win32 HWND and DXGI swapchain, but assert GPU readback, not screen
   capture or native menu focus. They are not Linux/headless/offscreen tests.
2. **Dedicated native-desktop lane:** propose an isolated, disposable Windows 11
   x64 VM/image, OS build >=26100, pinned image revision and nonzero UBR recorded,
   Graphics Tools installed, unlocked console desktop, DWM and usable display,
   runner launched in that interactive user's session rather than Session 0.
   Suggested custom labels: `[self-hosted, Windows, X64, cgpui-win11-desktop]`.
   These labels are a provisioning proposal, not existing runner inventory.
   Execute the complete 97-test DX12 suite and the shared renderer suite there.

`windows-2025` is an explicit image-family choice, not an immutable image digest
and not proof of Windows 11 desktop equivalence. Before implementing, confirm
that label is available for this repository and record `ImageOS`, `ImageVersion`,
runner image release, OS SKU/build/UBR and PowerShell version from the actual job.
A Windows Server image with the same build number does not establish client
Windows DWM/menu behavior. No hosted execution was performed in this audit.

Start the hosted lane as a genuine experiment whose failures remain failures;
promote it to required only after its real host passes. Keep a missing desktop
runner as **pending/not executed**, not a successful optional step. Never add
`continue-on-error`, blanket capability skips, or a conditional that omits all
GPU commands while returning a green acceptance check.

## Current repository facts

- [Phase G workflow](D:/Dev/Projects/cgpui/.github/workflows/phase-g-windows-linux.yml)
  currently uses `windows-latest`. Its Windows debug/release jobs package the SDK;
  driver fixtures do not exercise graphics. The separate examples-smoke job is
  the only Windows job that invokes software-Vulkan setup. There is no WARP,
  D3D12 debug-layer, GPU-validation, or full native-DX12 runtime lane here.
- [Dependency action](D:/Dev/Projects/cgpui/.github/actions/setup-phase-g-dependencies/action.yml)
  installs Vulkan SDK **1.4.341.1** with SHA-256 verification and checks its header
  and import library. [mcpp action](D:/Dev/Projects/cgpui/.github/actions/setup-mcpp/action.yml)
  pins **2026.11.21.1** and its archive digest. Reuse both; do not create a second
  ad-hoc SDK/toolchain download route. Neither action verifies Graphics Tools,
  WARP/SM6, native desktop availability, or CJK font fallback.
- [Software Vulkan setup](D:/Dev/Projects/cgpui/scripts/ci/windows-software-vulkan.ps1)
  selects Chrome's actual x64 SwiftShader manifest, sets `VK_DRIVER_FILES`, and
  requires a successful `vulkaninfo --summary` containing SwiftShader. Elevated
  execution also adds an HKLM ICD registration. This is explicitly intended for
  disposable hosted VMs, not a developer's persistent workstation. Archive the
  Chrome version, manifest, referenced DLL hash, and vulkaninfo output: Chrome's
  installed version is presently discovered, not pinned by this script.
- [DX12 test manifest](D:/Dev/Projects/cgpui/tests/renderer-dx12/mcpp.toml)
  currently registers **97** Windows cases. [Renderer manifest](D:/Dev/Projects/cgpui/tests/renderer/mcpp.toml)
  currently selects **92** cases on Windows, including backend-specific Vulkan
  tests. Running all 92 with `-Renderer dx12` does NOT turn all 92 into DX12 cases.
- [Debug setup](D:/Dev/Projects/cgpui/src/renderer/dx12/dx12_debug.cpp)
  calls `D3D12GetDebugInterface` and enables `ID3D12Debug1` GPU validation when
  requested; failure is a real error. Settings are process-wide and cached, so
  run debug modes in separate processes, never mutate them around live devices.
- [Adapter selection](D:/Dev/Projects/cgpui/src/renderer/dx12/dx12_adapter_selection.cpp)
  uses `EnumWarpAdapter` for exact `CGPUI_DX12_ADAPTER=warp`; failure does not fall
  back to hardware. [Common identity owner](D:/Dev/Projects/cgpui/tests/renderer/renderer_pixel_test_backend.cpp)
  logs actual backend/selection/name/vendor/LUID/software/fallback and validates
  software=true for a WARP request. An environment-variable print alone is not
  adapter evidence.

## Honest split of graphics and desktop requirements

| Group | Actual seam | Proposed initial placement |
| --- | --- | --- |
| Common 13 + `renderer_pixel_backend_identity_test` | Shown HWND + real swapchain + `request_pixel_capture` / readback; Latin/CJK and DirectWrite ink oracles | Hosted WARP candidate, no screen/menu acceptance claim |
| Native primitive, upload, synchronization, recovery tests | Mostly native HWND + native DX12/private resource assertions; some test-local scripts/failure sentinels | Full dedicated lane first; select a hosted subset only with an explicit reviewed allowlist |
| `dx12_runtime_root_recovery_test`, `dx12_runtime_child_recovery_test` | Real `run_app`, two ordinary windows, OS redraw/wakeup, real RemoveDevice and typed retry | Dedicated desktop lane initially; no assertion that service sessions suffice |
| All six `dx12_client_*` and eleven `dx12_ordinary_*` | OrdinaryDesktop/DWM and native-window state; positive alpha, physical RGB, menu/focus/control routes | Dedicated unlocked Windows 11 desktop |
| Popup pixels/backdrop_alpha/surface/platform/recovery/lifetime/primitives (seven cases) | Physical screen composition, work area/DPI, actual popup HWND behavior | Dedicated desktop lane |
| Popup backdrop_edges/layers (two cases) | Composition HWND + GPU capture, not physical screen sampling | Could later join an audited hosted subset, not automatically all popup tests |
| Loader isolation/independence | Fresh child processes; public DX12 route, poisoned-loader failure and mandatory healthy Vulkan controls | Full lane requires an actual healthy Vulkan loader AND ICD |

The seven `dx12_sync_*` cases use a real native device with injected sync results
but do not submit ExecuteCommandLists work; count them as sync-contract coverage,
not seven extra rendered WARP scenes.

Sources: [common HWND host](D:/Dev/Projects/cgpui/tests/renderer/renderer_pixel_test_window.cpp),
[common text oracle](D:/Dev/Projects/cgpui/tests/renderer/renderer_text_pixel_test.cpp),
[ordinary helper](D:/Dev/Projects/cgpui/tests/renderer-dx12/dx12_ordinary_test_support.hpp),
[menu pixel capture](D:/Dev/Projects/cgpui/tests/renderer-dx12/dx12_ordinary_menu_pixels_support.hpp),
[popup desktop helper](D:/Dev/Projects/cgpui/tests/renderer-dx12/dx12_popup_test_support.hpp),
[runtime loop](D:/Dev/Projects/cgpui/tests/renderer-dx12/dx12_runtime_recovery_loop_test_support.cpp),
[loader launcher](D:/Dev/Projects/cgpui/tests/renderer-dx12/dx12_loader_isolation_test.cpp).

Two existing zero-exit paths must be accounted for in CI results:

- Six client + nine ordinary cases call `ordinary_alpha_os_gate`: on build <26100
  they verify unsupported-platform refusal and print **SKIP ordinary alpha
  positive cases**. That is useful negative-contract coverage, NOT positive alpha
  coverage. Require build >=26100 and **no such SKIP** in the desktop lane.
- [Cross-adapter isolation](D:/Dev/Projects/cgpui/tests/renderer-dx12/dx12_coordinated_adapter_isolation_test.cpp)
  returns 0 with `SKIP: no independent SM6 adapter` when only WARP is available.
  Do not count this as cross-adapter acceptance. For a full native-desktop gate,
  provision one SM6 hardware adapter in addition to WARP and reject that skip.
  A software-only lane may report this one named case as not applicable with its
  reason, but must not claim “97 independently exercised native behaviors”.
  Do not weaken the owning test or remove this message to satisfy a job count.

## Prerequisite gate: fail clearly before treating a host as usable

Persist the preflight report even when it fails. Check in order:

1. Windows x64; exact OS product/build/UBR; selected runner image and host versions;
   mcpp release/digest, resolved MSVC and Windows SDK; Vulkan SDK version.
2. Query `Get-WindowsCapability -Online -Name Tools.Graphics.DirectX~~~~0.0.1.0`.
   Require `State=Installed`. For a newly provisioned ephemeral image, installation
   may use `Add-WindowsCapability -Online -Name Tools.Graphics.DirectX~~~~0.0.1.0`;
   a permission/network failure or `RestartNeeded` is a failed prerequisite,
   not a reason to unset debug flags. Prefer pre-provisioning/rebooting the pinned
   desktop image outside the test run. Do not install anything in this audit.
3. Record D3D12/debug DLL versions, but do not accept DLL existence as proof.
   Run the real identity test under WARP + DEBUG=1 + GPU_VALIDATION=1. Actual
   `D3D12GetDebugInterface`, `ID3D12Debug1`, device creation, shader/pipeline,
   capture and present must succeed. Source already fails on unavailable debug
   interfaces. This probe is the positive runtime gate, not a synthetic mock.
   Current general `dx12_check_debug_errors` returns success if InfoQueue Query
   fails; recovery helpers explicitly require InfoQueue when debug is requested.
   Do not claim every test independently certifies InfoQueue availability. The
   native setup path does require debug activation; add a focused InfoQueue
   availability preflight if stronger diagnostic completeness is a CI contract.
4. DirectWrite plus Segoe UI and distinct Chinese fallback glyphs are required by
   the unchanged text scene. Record available relevant fonts/image features;
   missing CJK coverage fails, rather than accepting two identical tofu boxes.
5. Full/both lanes must run healthy Vulkan preflight using the existing SDK plus
   software-Vulkan action on a disposable host, or a documented installed ICD on
   the dedicated image. SDK headers/import libraries are not a working driver.
   `dx12_loader_isolation_test` intentionally retries a poisoned loader and then
   executes a healthy-Vulkan child; `dx12_backend_selection_test` also requires
   automatic selection to create Vulkan. WARP cannot satisfy either control.
   If an explicitly restricted hosted native subset has no ICD, these two cases
   must be named as unexecuted and covered in the dedicated healthy-ICD lane,
   never recoded as successful skips.
6. Desktop lane additionally requires same active console user/session, DWM,
   unlocked input desktop, at least 500x380 visible work area for OrdinaryDesktop,
   no screen saver/lock,
   and no competing interactive jobs. Record SessionId, WTS session state,
   `SM_REMOTESESSION`, input-desktop availability and display geometry. Do not
   run the runner as a service. Avoid RDP attach/disconnect during the run;
   a disconnected/locked/minimized RDP desktop is not an acceptable substitute.
   The actual desktop color/menu/focus tests remain the final oracle even if
   `Environment.UserInteractive` and preliminary checks report success.

Desktop resources must be isolated: one runner/job at a time, fixed resolution,
DPI and theme, and no real-user secrets on that desktop. Do not execute arbitrary
fork PR code on a persistent authenticated self-hosted machine. Use disposable
VMs and a trusted approval route; never `pull_request_target` plus untrusted
checkout. Hosted common tests can retain normal unprivileged PR execution.

## Sequential commands and exact coverage

The following is a **future execution recipe**, not commands run by this audit.
Run from the repository root after setup. Use the existing annotation wrapper,
which tees complete stdout/stderr while preserving the actual command exit code.
All graphics test execution uses `MCPP_JOBS=1`; no parallel mcpp invocations or
cross-window test processes competing for the same desktop.

Hosted common lane, shell `bash` on Windows:

```bash
set -euo pipefail
export CGPUI_DX12_ADAPTER=warp CGPUI_DX12_DEBUG=1 CGPUI_DX12_GPU_VALIDATION=1
unset CGPUI_RENDERER
E="$RUNNER_TEMP/cgpui-warp-common"
mkdir -p "$E"
MCPP_JOBS=4 bash scripts/ci/run-with-github-annotation.sh "$E/build.log" -- \
  pwsh -NoProfile -File scripts/dev/test.ps1 --no-run -p tests/renderer
export MCPP_JOBS=1
cases=(
  renderer_pixel_backend_identity_test
  renderer_backdrop_blur_pixel_test renderer_box_shadow_pixel_test
  renderer_clip_pixel_test renderer_color_encoding_pixel_test
  renderer_frame_pixel_capture_test renderer_image_pixel_test
  renderer_opacity_pixel_test renderer_resize_pixel_test
  renderer_rounded_rect_pixel_test renderer_scale_pixel_test
  renderer_text_pixel_test renderer_text_ink_extent_pixel_test
  renderer_transform_pixel_test
)
for case in "${cases[@]}"; do
  bash scripts/ci/run-with-github-annotation.sh "$E/$case.log" -- \
    pwsh -NoProfile -File scripts/dev/test.ps1 -Renderer dx12 -p tests/renderer "$case"
done
```

The first identity case supplies the native positive preflight after compilation.
The per-case filter is a name filter, not proof of execution: a future focused
CI result checker must require the expected exact test name and one successful
execution in each log, no zero-match run, no FAIL, and no skip. Validate the
thirteen-name inventory against the owning Windows manifest, and require actual
`expected=dx12 actual=dx12 ... software=1 fallback=` evidence from their renderer
creation reports. Do not infer fourteen tests from fourteen exited processes.
Keep the independent `renderer_pixel_backend_identity_test` distinct from the
thirteen scene count. Full renderer-suite commands remain a separate 92-case gate.

Dedicated desktop lane, after successful desktop/Graphics Tools/Vulkan gates:

```bash
set -euo pipefail
export CGPUI_DX12_ADAPTER=warp CGPUI_DX12_DEBUG=1 CGPUI_DX12_GPU_VALIDATION=1
unset CGPUI_RENDERER
E="$RUNNER_TEMP/cgpui-warp-desktop"
mkdir -p "$E"
for suite in renderer-dx12 renderer; do
  MCPP_JOBS=4 bash scripts/ci/run-with-github-annotation.sh "$E/$suite-build.log" -- \
    pwsh -NoProfile -File scripts/dev/test.ps1 --no-run -p "tests/$suite"
done
export MCPP_JOBS=1
bash scripts/ci/run-with-github-annotation.sh "$E/native.log" -- \
  pwsh -NoProfile -File scripts/dev/test.ps1 -Renderer dx12 -p tests/renderer-dx12
bash scripts/ci/run-with-github-annotation.sh "$E/shared-both.log" -- \
  pwsh -NoProfile -File scripts/dev/test.ps1 -Renderer both -p tests/renderer
```

The second command means two independent mcpp runs (Vulkan then DX12); literal
`both` must never enter `CGPUI_RENDERER`. The Vulkan run still needs its real ICD,
and Vulkan-specific tests remain Vulkan in both runs. Assert the expected 97
native names and 92-per-run renderer inventory, not only the final summary.
Exact counts are current-snapshot facts; update the explicit inventory on a
reviewed test addition rather than accepting any positive count.

A cheap WARP/debug-only lane can precede GBV by removing GPU_VALIDATION in a new
process; GBV is not replaced by it. ASan is an additional scheduled/desktop gate:
run `-Asan --no-run` once, then `-Asan -Renderer dx12 -p tests/renderer-dx12` and
`-Asan -Renderer both -p tests/renderer` sequentially. Keep the current MSVC ASan
runtime/entry setup in `scripts/dev/test.ps1`; do not invent a second runner.
Do not equate ASan with D3D12 GPU validation. Record each mode separately.

## Logs, artifacts, timeouts, and failure policy

- Propose job budgets of 45 minutes hosted common and 60 minutes desktop initially;
  measured host performance must justify any adjustment. Give setup/build/runtime
  stages their own bounded budgets leaving time for failure artifact upload.
  Do not use “rerun until green” to hide a bad first attempt; retain first evidence.
- Retain existing [recovery watchdog](D:/Dev/Projects/cgpui/tests/renderer-dx12/dx12_recovery_test_support.hpp)
  (15 seconds, standalone process exits 90 on timeout) and
  [loader child deadline](D:/Dev/Projects/cgpui/tests/renderer-dx12/dx12_loader_process_support.hpp)
  (20 seconds per owned child plus bounded termination wait). GPU validation on
  a slower hosted CPU can expose these limits. Menu loops have a 4-second
  deadline and screen-color polling has a 2-second limit as well. A timeout is failure; only a
  separate evidenced test-budget change may adjust it, never CI skip conversion.
- For unbounded native tests, add an owning process-tree timeout wrapper only as
  a reviewed focused module; kill solely its own spawned process tree and
  preserve its logs/exit status. GitHub step/job timeout is the initial outer
  ceiling, not proof of graceful cleanup. Production native fence waits use
  INFINITE ([native sync owner](D:/Dev/Projects/cgpui/src/renderer/dx12/dx12_synchronization_native.cpp)); the test-only
  15-second watchdog is not a new production safe-release guarantee. Do not kill unrelated user/runner apps.
- `actions/upload-artifact@v4` must use `if: always()` and preserve setup,
  preflight, full per-command logs, inventory/result JSON, first failure, exact
  selected environment (allowlist only), host facts, dependency digests, source
  commit plus dirty-tree/source snapshot, and timing/exit/timeout statuses.
  Pre-create a small status artifact before setup; missing artifacts are an error,
  not a silent upload success. Never archive complete environment/secrets.
- Current pixel tests keep GPU captures in memory and print diagnostics; they do
  **not** automatically export PNG files. Do not advertise screenshot artifacts
  that do not exist. If needed later, add a focused failure-only capture exporter
  with its own tests; screen capture must be confined to the disposable test
  desktop/owned windows, not developer sessions.
- Classify failures as provisioning, build, runtime/assertion, missing-coverage,
  timeout, or artifact failure; all required categories remain nonzero. Debug
  error/corruption, absent capture, wrong backend, wrong software identity,
  missing test names and unexpected skips cannot count as acceptance.

## Minimal implementation modules after the freeze

No patch is supplied yet: host provisioning and experiment approval come first.
If approved, keep orchestration out of broad existing scripts:

- Focused `scripts/ci/windows-dx12-prerequisites.ps1`: host/Graphics Tools/desktop
  facts and explicit failures; no implicit disabling of requested debug mode.
- Focused inventory/result checker under `scripts/ci/`, with fixture tests for
  zero matches, missing names, skips, failure codes, and malformed identity.
- A short workflow job using existing dependency/annotation actions and the
  explicit inventory. Add structure checks for scope and registration rather
  than putting native diagnostic APIs in a large workflow shell block.
- Preserve normal Vulkan/package/UI regressions and do not change production
  automatic selection/default, window policy, or popup support in this CI slice.

## Evidence boundary and next decision

Local evidence under
`D:/Dev/shots/cgpui-dx12-common-parity-20261124/` includes `native-warp.log`,
`common-warp-both.log`, `asan-test.log`, and `asan-renderer-test.log`. These show
that the command surfaces have been exercised on the local Windows desktop;
this audit only inspected evidence and code, and did not reproduce them. They
are not hosted-runner evidence, and their source fingerprint must not be reused
for a subsequently changed tree. Root's later freeze/verification owns that claim.

Next decision: authorize one real hosted common-lane experiment and provision a
trusted dedicated desktop runner. Only a passing actual host run with complete
inventory/artifacts establishes that lane. Workflow registration alone never
equals host acceptance, hardware parity, native Windows compatibility closure,
performance readiness, or DX12 release completion.

Official reference entry points for implementation review (not fetched during
this offline/source-only audit): [GitHub runner images](https://github.com/actions/runner-images),
[GitHub hosted runners](https://docs.github.com/en/actions/using-github-hosted-runners/about-github-hosted-runners),
[Graphics Tools](https://learn.microsoft.com/en-us/windows/uwp/gaming/use-the-directx-runtime-and-visual-studio-graphics-diagnostic-features),
[GPU-based validation](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-d3d12-debug-layer-gpu-based-validation),
[WARP](https://learn.microsoft.com/en-us/windows/win32/direct3darticles/directx-warp).
Recheck current image availability and installation guidance before implementation;
repository source, not an assumed runner catalog, is the verified basis above.
