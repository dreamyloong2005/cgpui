# Acceptance: direct DX12 native Popup presentation

Date: 2026-11-23 (Asia/Shanghai). Baseline `5e28b484`, branch `phase-v`.
This milestone implements native alpha-composited Popups, not transparent
ordinary windows/native-menu coexistence or full DX12 parity.

## Delivered behavior

- Existing native Popup hosts use a DX12 command queue and RGBA8 premultiplied
  FLIP_SEQUENTIAL composition swapchain. A focused private DX12 host owns the
  DComp device/target/visual. There is no D3D11 rendering device, shared texture,
  keyed mutex, readback presentation or additional presentation copy on this
  path. Ordinary windows retain their opaque HWND FLIP_DISCARD path.
- The first successful presented/validated frame commits the visual. Resize
  keeps the same swapchain binding; native recovery rebuilds the host and resets
  first-frame commit state. Native teardown detaches/releases the host before
  swapchain/queue/device. Commit/detach is best effort during teardown; no
  unbounded compositor-completion wait was introduced. Actual external blockers
  still produce a bounded reconstruction error.
- Surface policy is a private leaf. The native descriptor must match the HWND's
  actual NOREDIRECTIONBITMAP style; invalid/destroyed HWNDs and topology mismatch
  return invalid_argument. Ordinary transparent creation/enabling transparency
  remains unsupported. Changing Popup transparent intent does not change its
  actual fixed premultiplied presentation, matching the existing Vulkan policy.
- ClearRTV receives premultiplied Popup clear colors; existing primitive blend
  paths retain premultiplied output. A new DX12-only backdrop composite samples
  the original scene and filtered scene and replaces RGBA through the rounded
  mask/effect opacity, with tint over the filtered scene. It neither drops alpha
  nor source-overs the same scene twice. The existing snapshot is reused, without
  new effect textures or constant layouts. This remains in-app blur, not blur of
  other desktop windows behind the Popup.
- Opaque parent DX12 renderers advertise Popup support. Existing runtime
  platform/renderer capability checks and actual initialization/placement failure
  cleanup remain in force. Pointer-transparent Tooltip hosts and nonactivating
  menu-style Popup hosts use the existing platform path unchanged.

The adopted popup policy retains internal DComp and forbids an application-facing
native/composition window-mode choice. This milestone adds no such public mode.
Vulkan/default/auto and failure-only Overlay policies are unchanged; explicit
DX12 never silently switches backend or adapter. Existing native HMENU is not
globally disabled; ordinary transparent-window/menu coexistence remains future
work. No release, default switch or performance-parity claim.

## Verification

Evidence root: `D:/Dev/shots/cgpui-dx12-popup-20261123/`.
Both final and native serial drivers exited 0. Root owned all builds and tests;
no-run builds used MCPP_JOBS=4, native execution used MCPP_JOBS=1.

| Check | Result | Evidence |
| --- | --- | --- |
| Public desktop Popup tracer RED / GREEN | expected exit4 / PASS | `popup-red.log`, `popup-green.log` |
| Transparent backdrop regression RED twice / GREEN | expected exit13 twice / PASS | `backdrop-alpha-red.log`, `backdrop-alpha-red-repeat.log`, `backdrop-alpha-green.log` |
| Expanded WARP + Debug native suite | 53 passed, 0 failed | `native-complete-test.log` |
| Final invalid-HWND/intent/resize tracer | PASS | `surface-final-test.log` |
| Complete architecture suite | 204 passed, 0 failed | `architecture-initial-test.log`, `windows-final-test.log` |
| Frozen Windows complete suite | 717 passed, 0 failed | `windows-final-test.log` |
| Windows all libraries/examples | PASS | `windows-all-build.log` |
| Windows installed-only SDK compile/link/run | PASS | `installed-sdk-windows.log` |
| Frozen Linux/WSL complete suite | 637 passed, 0 failed | `wsl-full-test.log` |
| Linux/WSL all libraries/examples | PASS | `wsl-all-build.log` |
| Linux installed-only SDK compile/link/run | PASS | `installed-sdk-linux.log` |
| DX12 ASan + WARP + Debug | 53 passed, 0 failed | `asan-dx12-test.log` |
| Intel Graphics + Debug | 53 passed, 0 failed | `native-0.log` |
| NVIDIA RTX 5070 Ti Laptop + Debug | 53 passed, 0 failed | `native-1.log` |
| WARP + Debug + GPU validation | 53 passed, 0 failed | `native-warp.log` |
| Independent public headers / neutral renderer | 47 / 15 passed | `windows-final-test.log` |
| SDK graph/header fixtures | 4 + 15 passed | `sdk-graph-fixtures.log`, `sdk-headers-fixtures.log` |
| DXIL exact regeneration check | PASS | `dxil-check.log` |
| Existing Vulkan backdrop payload | all 2873 SPIR-V words unchanged | `shader-scope.json` |
| Windows frozen-source comparison | all 2727 files match | `source-match.json` |
| WSL frozen-source comparison | all 2727 manifest paths match after sync normalization | `wsl-source-match.json` |

The installed Windows SDK consumer exercises ordinary drawing plus hidden Popup
creation, alpha-aware blur and RGBA capture through installed headers/static
libraries. It proves link closure and execution, not desktop transparency; desktop
evidence is provided separately by the native tests. Linux's consumer validates
the installed link/run and invalid-surface error path, not native DX12 rendering.

The source manifest excludes generated build outputs, dated planning notes and
the user's pre-existing `.vscode/`. Windows compares raw SHA256 and detects added,
removed or changed source paths. WSL independently verifies archived bytes against
the manifest, then checks every manifest path using the sync rule: CRLF becomes
LF only for data containing no NUL. Unlisted build outputs are not scanned.
Neither comparison found changes or missing files. Only closeout planning notes
changed after the final source freeze.

## New test coverage

Nine native programs are registered:

1. Alpha 0/0.5/1 rectangles over a controlled green backing HWND: both texture
   and actual desktop pixels, then red-to-blue updates rejecting stale visuals.
2. Transparent/half-transparent uniform backdrops, effect opacity 1/0.5 and tint:
   literal RGBA plus desktop checks. The original failure was texture alpha 255
   and a real desktop black rectangle over an all-zero transparent scene.
3. Nonblack clear premultiplication, true/false intent reports, invalid hosts,
   actual HWND resize plus renderer DPI 2-to-1 updates and continuing frames.
4. Real PlatformApplication Popup and pointer-transparent Tooltip hosts: opaque
   DX12 parent capability, alpha pixels, placement, ownership/styles, nonactivation
   and WindowFromPoint input pass-through. This is native host verification, not
   an exhaustive interactive Tooltip/Popover widget acceptance.
5. Two idle-device removal epochs, first Popup alone then its real opaque owner
   coordinating recovery. Images are uploaded once; active/finished frames and
   borrowed capture lifetimes are checked. Alpha backdrop resources also exist
   before removal. Post-recovery desktop colors change to reject stale visuals.
6. Renderer-before-frame and destroyed-HWND orphan lifetimes, same-HWND target
   reuse, duplicate factory rejection without corrupting the existing host, and
   recovery proving partial initialization/old visual references do not block it.
7. Mixed image, independently shaped native text, rounded rect and analytic
   Gaussian shadow samples; full-frame premultiplied bounds and desktop samples.
   Glyph checks prove coverage and transport, not a fixed font-outline oracle.
8. Independent Gaussian CDF comparisons across sigma 4/12/32/64 and opacity 1/0.5
   on nonuniform red/transparent edges, including reduced formerly opaque alpha.
9. Fractional straight/rounded mask edges, clipping, overlapping backdrops with
   intervening draws and later foreground, using literal ordered-RGBA expectations.

All three non-ASan native matrix logs explicitly exercise distinct hardware/WARP
LUID isolation, with no SKIP. ASan's custom process exit can suppress buffered
stdout; its passing program count is not used alone as a no-skip claim. Native
removal tests call RemoveDevice on idle devices, not physical resets or arbitrary
in-flight TDR. DPI tests change renderer scale, not physical monitor migration.
Desktop tests cover this machine's actual compositor configuration; they do not
certify every desktop, HDR/remote-session configuration or native macOS.

## Reviews and remaining limits

### Standards

No hard violations or blocking lifetime findings. One nonblocking possible
duplicate-code smell in opaque/alpha shader sampling was accepted to preserve
existing opaque/Vulkan compiled instructions in this slice. No module-size caps
were raised. Structure tests protect direct hosting, policy ownership, alpha
replacement binding/retirement and absence of the D3D11 sharing bridge dependency.

### Spec

Zero actionable findings. Independent native-lifetime review confirmed partial
initialization, visual/swapchain release order, peer operation guards and fresh
first-frame commit after recovery. Its requested invalid-host regression was
added and passed before freezing. Ordinary transparency and native-menu coexistence,
cross-thread recovery, diagnostics/performance parity and release/default-switch
gates remain unfinished.

Shader regeneration initially touched unrelated Metal outputs because this
host's Slang version differed; only those agent-generated changes were restored.
Metal files are unchanged. Existing Vulkan SPIR-V and old DXIL entry payloads
are unchanged; the new entry is DX12-only. The initial test link failure was a
missing test-only platform-win32 dependency, corrected without a product stub.

User `.vscode/`, the separate design checkout and user's Linux checkout remain
untouched. Only the dedicated WSL validation mirror was synchronized. Local
commit only; no remote push.
