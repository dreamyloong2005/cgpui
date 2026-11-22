# Popup presentation policy revision

Date: 2026-11-22
Status: implemented as the first policy slice; supersedes the public window-mode
proposal in the initial DX12 plan and the subsequent blanket Windows/Vulkan
popup refusal. This document scopes the first corrected implementation
slice, not completion of the DX12 backend.

## Decision

Application code describes window intent: background, decorations, popup
placement and menus. Do not add `WindowPresentation`, a `presentation`
field, or a native/composition option to `WindowDescriptor`/`WindowOptions`.
DirectComposition is an implementation mechanism, not an application
window kind. Presentation paths stay private to the platform and renderer.

On Windows, Vulkan retains independent self-drawn popup windows through the
existing Vulkan -> shared D3D11 texture/keyed mutex -> copy to DComp
composition swapchain path. With their default popup preference, Tooltip
and PopoverMenu attempt this path when both platform and renderer support
it. Do not preemptively force Windows/Vulkan to use an overlay.

The existing in-window overlay is used only when capability is genuinely
unavailable, popup creation/renderer initialization/placement actually fails,
or the widget's existing explicit popup preference is disabled. Fallback
retains input and command handling but cannot extend past the parent window.
Native context menus remain available and are not gated by the renderer
popup capability.

DX12 is not implemented yet. Its planned transparent popup path retains
private DirectComposition hosting with a DX12 composition swapchain,
without the Vulkan/D3D11 shared-texture copy. Ordinary HWND presentation
alone is not a verified replacement for popup per-pixel alpha. No public
window-mode switch or silent removal of native menus may be introduced to
work around this. Transparent normal windows, dynamic backgrounds and
native menu coexistence require explicit implementation and validation in
the DX12 work; the current Vulkan behavior is not changed by this slice.

## Ownership and contract

- `Renderer::popup_windows_supported()` is a neutral capability, not a
  backend selector. The base implementation returns false in a focused
  renderer source; custom renderers explicitly opt in. Vulkan and Metal
  retain support. Every renderer still needs a popup-capable platform;
  there is no Windows/Vulkan blacklist.
- `PlatformApplication::popup_windows_supported()` describes only native
  window support. A successful Win32 DComp probe is not sufficient to
  advertise application popup support.
- The runtime intersects platform support and the active parent renderer
  capability. Before a renderer exists it reports false. The policy lives
  in a focused private runtime module, not in widgets or broad headers.
- Both `open_popup` and direct popup `WindowOptions` creation must enforce
  the policy. Unsupported requests return `unsupported_platform` through
  result-returning interfaces, or record the error for legacy `open_window`.
  Supported requests attempt independent popup creation, then validate the
  created popup renderer. Rejected or failed requests must clean up partial
  windows and owned views without leaving an active popup. Widget fallback
  also covers actual renderer initialization and placement failures.
- Ordinary child windows are unaffected. Existing supported popup input,
  placement, closing and keyboard forwarding remain covered by tests.
- Low-level Win32/DComp and Vulkan interop code and tests are retained in
  this slice and remain the supported Windows/Vulkan runtime popup path.
  Their per-popup device setup and per-frame copy/keyed-mutex costs remain
  optimization risks, not a reason to disable the path unconditionally.

## Scope exclusions

Do not change Vulkan GPU selection or existing normal-window transparency,
backdrops, native menu installation, or macOS popup behavior. Do not remove
DComp based on an unverified DX12 transparency assumption. Do not introduce
DX12 stubs that pretend to render or enable a partial backend by default.

## Evidence

Upstream GPUI uses DComp internally on Windows but has no public
native/composition window choice. Its Windows menu behavior is not proof
that CGPUI's existing native HMENU can be preserved with every composition
topology. See the accompanying GPUI research note in the design worktree.

A standalone local probe tested 32 combinations of Intel/NVIDIA,
FLIP_DISCARD/FLIP_SEQUENTIAL and HWND/DWM styles on Windows 11 build 26300.
All displayed alpha-zero as black and half-red without the green background
contribution, despite successful calls. This rules out the tested recipes,
not all possible non-DComp implementations. Evidence and source:
`D:/Dev/shots/dx12-alpha-no-dcomp-20261122/README.md` and `alpha.cpp`.

## Acceptance

1. Capability matrix covers platform refusal, renderer refusal, absent
   renderer, and both supporting the feature. Direct window creation
   cannot bypass the same policy; normal child windows still work.
2. Tooltip and PopoverMenu default popup preference attempts independent
   windows on supported success, and yields a visible, usable overlay only
   after genuine refusal or actual creation/renderer/placement failure.
   Their existing explicit popup opt-out still selects the overlay.
3. Real Windows Vulkan and Metal keep their opt-in. Windows policy does not
   silently disable other platforms; failed popup attempts leave no active
   popup or leaked owned view.
4. Structure guards keep policy in leaf files and prevent a public
   native/composition window mode. Existing composition tests stay green.
5. Run targeted tests, full Windows regression, and available Linux
   regression. Report native macOS validation separately and honestly.
