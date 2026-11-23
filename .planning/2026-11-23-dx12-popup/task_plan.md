# DX12 direct native Popup presentation

Baseline `5e28b484`, branch `phase-v`. User requested continuation. Preserve the
user's `.vscode/`, design checkout and Linux checkout. Local commit, no push.

## Authoritative scope

Follow the already adopted policy in
`docs/superpowers/plans/2026-11-22-popup-presentation-policy-revision.md`:
private DirectComposition hosting is retained; applications select window intent,
not native/composition modes. Vulkan remains default (including auto), Vulkan
Popup remains enabled, and Overlay fallback follows genuine failure/unsupported
capability. Explicit DX12 never silently switches renderer or adapter.

Implement DX12 direct composition swapchain for the existing native Popup host.
Do not use Vulkan/D3D11 texture sharing, readback presentation or public modes.
Ordinary transparent HWNDs and their native-menu coexistence remain a separate
milestone: the previous 32-case HWND probe did not demonstrate per-pixel alpha.

## Ownership and seams

- DX12 composition host/creation/retirement stays in focused private DX12 leaf
  modules. It is not added to broad platform/application code. Existing Vulkan
  interop is unchanged; native lifetime differs from its shared-texture bridge.
- Renderer surface policy owns intent validation, reports and capability.
  Submission reuses existing drawing/capture, with premultiplied clear and a
  first-frame composition commit after successful Present.
- Device loss/resize/destruction must retire visual references to old swapchains,
  rebuild on the same adapter and preserve bounded recovery semantics.
- Transparent backdrop composition needs correct premultiplied replacement,
  not another source-over of the original scene's alpha. Do not advertise success
  based solely on rectangles or transparent-black samples.
- Use existing approved public native test seams: factory/Renderer/RenderFrame,
  PlatformApplication Popup creation, captured pixels plus actual DWM desktop
  pixels over a controlled green backing window. No mock composition success.
- Root runs all builds/tests serially (MCPP_JOBS=4 no-run, 1 execution), owns
  registrations, surface policy, integration, structure tests and acceptance.
  No source-size cap increases. Agents do not build or commit.

## Sequence

1. [done] Public real-desktop Popup tracer RED.
2. [done] Direct DX12+DComp Popup rendering GREEN and surface policy.
3. [done] Transparent clear/effect, native platform, resize/lifetime/recovery
   regressions and structure guards.
4. [done] Independent Standards/Spec/native lifetime review; full frozen
   Windows/WSL, native adapters, SDK and scoped ASan validation.
5. [done] Acceptance recorded; local commit closes the milestone, no push.

Evidence: `D:/Dev/shots/cgpui-dx12-popup-20261123/`.
