# Window backdrop material

CGPUI can ask the platform to draw a system material behind a window: Mica,
Mica Alt (tabbed), or Acrylic on Windows 11, and compositor blur on Wayland.
The material is only visible through the client area when the renderer's
swapchain composites with alpha, so the window, the renderer, and the
runtime each report what they actually did.

## API

`include/cgpui/core/window_backdrop.hpp`:

```cpp
enum class WindowBackdrop { none, mica, mica_alt, acrylic, blur };
```

`WindowDescriptor::backdrop` (default `none`) requests it at creation;
`WindowOptions::backdrop(WindowBackdrop)` sets it from the builder.

`include/cgpui/platform/platform_window_backdrop.hpp`:

```cpp
struct PlatformWindowBackdropState {
  bool supported = false;            // backend has a mechanism for this window
  bool transparent_surface = false;  // renderer surface composites with alpha
  std::string backend;               // "win32", "wayland", "unsupported"
  WindowBackdrop requested = WindowBackdrop::none;
  WindowBackdrop applied = WindowBackdrop::none;    // behind the client area
  WindowBackdrop title_bar = WindowBackdrop::none;  // system title bar (Windows)
  std::string mechanism;             // e.g. "DWMWA_SYSTEMBACKDROP_TYPE=2"
  std::string reason;                // why `applied` is none
};
```

`include/cgpui/renderer/renderer_surface_composition.hpp`:

```cpp
enum class RendererCompositeAlpha {
  unknown, opaque, pre_multiplied, post_multiplied, inherit };

struct RendererSurfaceComposition {
  bool supported = false;
  bool transparent_requested = false;
  std::uint32_t supported_composite_alpha = 0;  // raw VkCompositeAlphaFlagsKHR
  RendererCompositeAlpha composite_alpha = RendererCompositeAlpha::unknown;
  constexpr bool alpha_composited() const;      // pre- or post-multiplied
};
```

Entry points:

| Layer | Set | Query |
| --- | --- | --- |
| `PlatformWindow` | `apply_window_backdrop(WindowBackdrop, bool transparent_surface)` | `window_backdrop_state() const` |
| `Renderer` | `set_transparent_surface(bool)` (backend-dependent; may recreate a swapchain or only update intent) | `surface_composition() const` |
| `Window` (app facade) | `set_backdrop(WindowBackdrop) const` | `backdrop() const`, `surface_composition() const` |
| `WindowOptions` | `backdrop(WindowBackdrop)` | `descriptor.backdrop` |

`App` reaches a window through `App::root_window()` and `App::window(id)`.
`Window::descriptor().backdrop` stays the creation-time request; the live
state comes from `Window::backdrop()`. The base `PlatformWindow` and
`Renderer` report `supported = false` with a reason (X11, test
platforms, headless renderers).

```cpp
auto window = app.open_window(
    cgpui::WindowOptions{}.title("Editor").backdrop(cgpui::WindowBackdrop::mica));
const cgpui::PlatformWindowBackdropState state =
    app.window(window.runtime_id)->backdrop();
if (state.applied == cgpui::WindowBackdrop::none) log(state.reason);
```

## How the runtime applies it

1. The platform window applies the request at creation with
   `transparent_surface = false`, because no renderer exists yet. On Windows
   this already sets the title bar material.
2. The runtime requests a transparent renderer surface for explicit
   `chrome.transparent_background`, a non-`none` backdrop, or a Popup.
3. `runtime_apply_window_backdrop` (src/ui/runtime_window_backdrop.cpp)
   re-applies the backdrop with the renderer's real composition. If the
   backend reports `supported` and the surface is opaque, it calls
   `Renderer::set_transparent_surface(true)` once and applies again with the
   new composition. `Window::set_backdrop` uses the same path at runtime.
4. Root and secondary windows use the same clear policy. Explicit
   `chrome.transparent_background` clears to `(0, 0, 0, 0)` when the renderer
   currently reports `supported` and pre-/post-multiplied alpha, even with
   backdrop `none`. Unknown, opaque, inherit or unsupported reports do not
   satisfy that explicit-transparency branch. Otherwise an actually applied
   material (`applied != none` and `transparent_surface`) requests transparent
   clear; with neither, the usual opaque `(0.08, 0.09, 0.10, 1)` remains.
   Popups retain their transparent clear for rounded corners and shadows.
   The `render_view(..., Color clear_color, ...)` overload carries the color.
   Content that paints an opaque root background still hides the material or
   desktop; leave the root transparent to see it.

Switching to `none` keeps the transparent swapchain. A material-only window's
clear turns opaque again; an explicitly transparent window keeps transparent
clear while its renderer actually supports alpha. The creation-time backdrop
in `Window::descriptor()` does not override the live material state. This
runtime policy does not add alpha capability to a backend or platform. DX12 now
supports a bounded initially transparent ordinary-window route described below;
it still refuses dynamic upgrades from an opaque ordinary renderer.

## Vulkan composite alpha

`choose_vulkan_composite_alpha` (src/renderer/vulkan/vulkan_surface_selection.cpp)
reads `VkSurfaceCapabilitiesKHR::supportedCompositeAlpha` each time the
swapchain is created. For a transparent request it prefers `PRE_MULTIPLIED`,
then `INHERIT`, else `OPAQUE`. `POST_MULTIPLIED` is no longer chosen: the
pipelines blend with `SRC_ALPHA / ONE_MINUS_SRC_ALPHA` for color and
`ONE / ONE_MINUS_SRC_ALPHA` for alpha, which produces premultiplied output.
For an opaque request the order is unchanged (`OPAQUE` first).

`RendererSurfaceComposition::alpha_composited()` is true only for pre- or
post-multiplied. `INHERIT` leaves alpha to the native window, which CGPUI
cannot confirm, so windows treat an inherit surface as opaque and report
`applied = none`.

Color encoding: `Color` components are sRGB-encoded values and the
pipelines blend them in that space. `choose_vulkan_surface_format` prefers
`B8G8R8A8_UNORM`, then `R8G8B8A8_UNORM`, with
`VK_COLOR_SPACE_SRGB_NONLINEAR_KHR`, so the swapchain stores the values
unchanged (Metal uses `BGRA8Unorm` the same way). Image textures
(`R8G8B8A8_UNORM`) and the glyph coverage atlas (`R8_UNORM`) are sampled
as stored. Until 2026-11-17 the renderer preferred `B8G8R8A8_SRGB`, which
encoded the values a second time: rgb(32,34,38) was shown as
rgb(99,102,108) and the default clear 0.08/0.09/0.10 as rgb(80,84,89).
After the change the example's caption row is captured from the screen as
rgb(32,34,38), the command box as rgb(52,55,61), and the clear as
rgb(20,23,25). Text edge coverage is now blended in sRGB-encoded space
rather than linear space, so anti-aliased edges of light text on a dark
background look slightly thinner; the Vulkan text pixel regression passed at
that milestone. Its current shared scene is `renderer_text_pixel_test`, with
explicit Vulkan/DX12 selection as described in [DX12 testing](dx12-backend.md#common-windows-pixel-contracts).

## Windows

Source: src/platform/win32/win32_window_backdrop_plan.cpp (decision),
win32_window_backdrop.cpp (DWM calls), win32_window_backdrop_window.cpp.

| Request | `DWMWA_SYSTEMBACKDROP_TYPE` (38) | `applied` |
| --- | --- | --- |
| `none` | `DWMSBT_NONE` (1) | `none` |
| `mica` | `DWMSBT_MAINWINDOW` (2) | `mica` |
| `mica_alt` | `DWMSBT_TABBEDWINDOW` (4) | `mica_alt` |
| `acrylic` | `DWMSBT_TRANSIENTWINDOW` (3) | `acrylic` |
| `blur` | `DWMSBT_TRANSIENTWINDOW` (3) | `acrylic` |

`blur` maps to acrylic: DWM has no documented generic blur-behind for
Windows 11 top-level windows other than the acrylic system backdrop
(`DwmEnableBlurBehindWindow` does not blur on Windows 8 and later).

Decision order:

1. The build comes from `RtlGetVersion` (GetVersionEx lies without a
   manifest). Below 22621: `supported = false`, DWM is not touched.
2. `SystemAppearance::transparency_effects_enabled == false`: the type is set
   to `DWMSBT_NONE`, `applied = none`, reason "transparency effects are
   turned off in Windows settings".
3. The renderer surface is not alpha-composited (opaque or inherit): the
   type is set, so DWM draws the material on the system title bar
   (`title_bar = <material>`), the frame is not extended, `applied = none`,
   and the reason says the material is on the title bar only. In client-title-bar
   mode there is no visible system title bar, so `title_bar = none` instead.
4. Otherwise the type is set and `DwmExtendFrameIntoClientArea` gets margins
   of -1, so the material extends behind the whole client area;
   `applied = title_bar = <material>`. A failing DWM call turns `applied`
   back to `none` with the call named in `reason`. Every decision that
   touches DWM also sets the margins: -1 when covering the client area, 0
   otherwise.

`chrome.transparent_background` sets no window style. It only asks the
renderer for a transparent surface, so it combines with every backdrop.
Until 2026-11-22 it also made the window `WS_EX_LAYERED` with alpha 255,
and the decision refused backdrops on layered windows. A screen read-back
over a green window, with frames cleared to alpha 0, found no other effect
of the layered style: on the NVIDIA GPU (`PRE_MULTIPLIED`) the cleared
area showed the green window with and without it, on the Intel GPU
(`INHERIT`) it was black in both cases, and `WindowFromPoint` returned the
window in both cases. With Mica, the NVIDIA window now shows the material
(rgb(32,32,32) on this machine).

Re-evaluation: each window's `Win32WindowBackdrop` registers itself as the
window property `CGPUIWindowBackdrop`. The existing appearance monitor
(`CGPUIAppearanceMonitor`), after refreshing the dark title bars, calls
`win32_reevaluate_thread_window_backdrops(snapshot)`, which re-runs the
decision for every CGPUI window on the thread with the new
`transparency_effects_enabled`. No second monitor or callback exists. The
runtime bridge then requests a render when `changed` is true, and the next
frame picks the clear color from the new state. Without a runtime callback
installed, the DWM state still follows; the clear color updates on the next
frame.

Redirection surface: with the frame extended, DWM composites the window's
GDI redirection surface under the transparent swapchain. Before the backdrop
erase handler, the class's null background brush and lack of GDI client painting
left that surface opaque white, covering the material. While the frame is extended,
`win32_window_proc_background.cpp` answers `WM_ERASEBKGND` by filling the
client black (GDI black has alpha 0, which DWM treats as transparent in an
extended frame), and `apply_plan` invalidates with `RDW_ERASE` whenever it
changes the margins so the surface is repainted at once. Windows without an
extended frame keep the null brush and the default handling, except for the
independent DX12 ordinary-alpha lease below.

### Experimental DX12 ordinary windows

Windows build **26100+** can use DX12 initial ordinary transparency on a
CGPUI-owned, decorated window with a visible system or client-drawn title bar.
System-title-bar windows can retain their native application HMENU; client-title-bar
windows require **no attached application HMENU bar**. The real DWM alpha
attribute must succeed. A creation-time backdrop already makes the runtime
request initial renderer transparency; explicit `chrome.transparent_background`
does likewise. There is no application-facing DComp/native window-mode choice.
Raw HWNDs, hidden title bars, undecorated windows and older OS versions remain
unsupported; Vulkan/default selection and Popups are unchanged. An active alpha
lease fixes its initial title-bar mode and rejects fullscreen, hidden or
undecorated transitions. See [DX12 backend scope](dx12-backend.md#bounded-ordinary-window-transparency).

The alpha lease owns `DWMWA_REDIRECTIONBITMAP_ALPHA` independently of this
material module's backdrop type and frame margins. Its redirected client remains
alpha-zero black even when the material is `none`. Actual composition stays
premultiplied after `set_transparent_surface(false)`; that call changes intent,
not topology. The runtime still chooses an opaque clear when neither explicit
transparency nor an applied material requires a transparent clear.

An ordinary renderer created opaque keeps its old HWND flip swapchain.
`set_backdrop(mica)` cannot upgrade that renderer to alpha; a system title bar can
show a title-bar-only material, not an applied client material. An opaque client
title bar reports no visible title-bar material. Destroying the opaque renderer
does not erase the HWND's flip history or permit later alpha creation.
Request a needed material/transparent surface at window creation instead.

On the supported **system-title-bar** route, menus remain actual Windows HMENUs.
The platform preserves the native
menu RGB and repairs only its alpha through nonclient buffered painting after
native events; it does not replace the menu with framework drawing. On
2026-11-23, `dx12_ordinary_backdrop_menu_test` observed `mica` actually applied,
premultiplied GPU pixels intact and **zero differing native menu-bar RGB pixels**
before/during/after a real command loop against the same window's opaque
reference. Removing Mica restored green-backdrop desktop alpha with a newly
rendered blue frame and again exact menu pixels. This is one-host Mica/none
coexistence evidence, not a Mica Alt/Acrylic or physical mouse-hover claim.

For client-title-bar alpha, initial creation with an application HMENU is refused
without hiding/replacing it. Nonempty application menu installation is rejected
transactionally while any client-alpha lease is active; install/clear callbacks
cannot acquire a new client-alpha lease or reenter menu installation. The native
system/window menu is unaffected. The pre-existing opaque client/HMENU layout
defect remains unresolved; this is not native application-menu support for client
title bars. `dx12_client_alpha_lifetime_test` checks no-menu client alpha through
Mica and `none`: it accepts an explicit OS/policy material refusal with a reason,
but always requires premultiplied GPU pixels and restored desktop alpha after
`none`. The test does not claim Mica is applied on every system.

## Wayland

Source: src/platform/linux/wayland_backdrop_globals.cpp (binding and
decision), wayland_window_backdrop.cpp (per-surface objects). The protocol
code is hand-written like the other protocols in src/platform/linux (the
repository does not run wayland-scanner):
wayland_protocol_background_effect_interfaces.cpp and
wayland_protocol_background_effect.cpp follow
`staging/ext-background-effect/ext-background-effect-v1.xml` (wayland-protocols)
and `src/protocols/blur.xml` (plasma-wayland-protocols), version 1 of each.

- The registry binds `ext_background_effect_manager_v1` and
  `org_kde_kwin_blur_manager` only when advertised. After binding the ext
  manager the application does one extra roundtrip to receive its
  `capabilities` event.
- Protocol choice: ext when its `blur` capability bit is set, else KDE blur,
  else none.
- All non-`none` requests map to `blur` (`applied = blur`), with `mechanism`
  `ext_background_effect_v1` or `org_kde_kwin_blur`.
- ext: one `ext_background_effect_surface_v1` per surface;
  `set_blur_region` with a region of `(0, 0, INT32_MAX, INT32_MAX)` (the
  compositor clips it to the surface, so it survives resizes);
  `set_blur_region(NULL)` removes it. KDE: `create` + `set_region(NULL)`
  (whole surface) + `commit`; removal is `unset` + `release`. Each change is
  followed by `wl_surface.commit` and a flush.
- No protocol: `supported = false`, reason names both globals, the renderer
  is never switched, so the surface stays opaque.
- Opaque surface with a protocol: `applied = none` with a reason.
- `SystemAppearance::transparency_effects_enabled` is true exactly when a
  blur protocol is usable, in `system_appearance()` and in every change
  callback (see docs/system-appearance.md).
- Capability changes after startup are stored but not pushed to existing
  windows; re-apply to pick them up.

## macOS

Source: src/platform/macos/macos_window_backdrop.mm (decision and view),
src/renderer/metal/metal_surface.mm (composition).

| Request | `NSVisualEffectView.material` | `applied` |
| --- | --- | --- |
| `none` | no view | `none` |
| `mica` | `windowBackground` | `mica` |
| `mica_alt` | `underWindowBackground` | `mica_alt` |
| `acrylic` | `popover` | `acrylic` |
| `blur` | `sidebar` | `blur` |

- The view uses `blendingMode = behindWindow` and
  `state = followsWindowActiveState`. It becomes the window's content view,
  and CGPUI's content view (with the `CAMetalLayer`) is its only subview,
  resized with it. First responder, input and the framebuffer size are
  unchanged. `none` puts CGPUI's view back as the content view.
- Decision order: `none` removes the view; Reduce Transparency
  (`accessibilityDisplayShouldReduceTransparency`) gives `applied = none`
  with a reason; an opaque renderer surface gives `applied = none` with a
  reason; otherwise the material is shown.
- While a material is requested, the window observes
  `NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification` and re-runs
  the decision, then requests a redraw so the clear color follows.
- Metal: `Renderer::set_transparent_surface` sets `CAMetalLayer.opaque`
  (no swapchain to recreate). A non-opaque layer composites premultiplied
  alpha, which the pipelines produce, so `surface_composition()` reports
  `pre_multiplied`; `supported_composite_alpha` is a Vulkan mask and stays 0.
  `apply_window_chrome` keeps the layer non-opaque while a material is shown.
- Tests: `macos_window_backdrop_test` (decision table, view hierarchy, first
  responder, material switch, removal); `metal_transparent_surface_test`
  (composition report, a cleared pixel captured as alpha 0 and a
  half-transparent red rect as premultiplied `128, 0, 0, 128`).
- Observed 2026-11-21 (macOS 27.2.1, one 1x display): `example_workbench`
  with `CGPUI_EXAMPLE_BACKDROP=1` opens and draws, but its root paints an
  opaque background, so the material is not visible through it; no pixel
  of the material itself was measured.

## What this machine reports (2026-11-17)

Windows 11 Home China, build 26300 (`RtlGetVersion`; UBR 9974), so
`DWMWA_SYSTEMBACKDROP_TYPE` is available. Transparency effects were enabled.

`supportedCompositeAlpha` from `vulkaninfo` (Vulkan SDK 1.4.341.1 on
Windows; vulkan-tools 1.4.357 in WSL):

| Surface | Device | Mask | Bits |
| --- | --- | --- | --- |
| `VK_KHR_win32_surface` | Intel(R) Graphics (three ICD entries) | 0x9 | OPAQUE, INHERIT |
| `VK_KHR_win32_surface` | NVIDIA GeForce RTX 5070 Ti Laptop GPU | 0x3 | OPAQUE, PRE_MULTIPLIED |
| `VK_KHR_wayland_surface` (WSLg) | Dozen on NVIDIA, Dozen on Intel, llvmpipe | 0x3 | OPAQUE, PRE_MULTIPLIED |
| `VK_KHR_xcb_surface` (WSLg XWayland) | same three | 0x9 | OPAQUE, INHERIT |

Device selection keeps enumeration order, except that a renderer created
with `transparent_background = true` prefers the first suitable device whose
surface supports `PRE_MULTIPLIED` (`vulkan_device_preference.cpp`). The
runtime requests a transparent surface at creation when the window
descriptor sets a backdrop or `chrome.transparent_background`
(`runtime_wants_transparent_surface`).

Before that preference existed, the renderer took the Intel GPU (mask 0x9),
chose `INHERIT`, and reported `applied = none`, `title_bar = mica`.
`win32_vulkan_backdrop_test` now creates a real window with `backdrop = mica`
and a transparent renderer: it selected the NVIDIA device (mask 0x3),
composited with premultiplied alpha, and `apply_window_backdrop` reported
`applied = mica` with no reason. A backdrop requested only at runtime, on a
window created without one, keeps the device chosen at creation; on this
machine that is Intel, so it still gets the title bar only. Preferring the
discrete GPU costs power on laptops; request a backdrop only when it is
wanted.

`win32_window_backdrop_test` read back `DWMWA_SYSTEMBACKDROP_TYPE` = 2 after
`mica` (title bar at creation, then client area with
`transparent_surface = true`), 1 after a synthetic
`transparency_effects_enabled = false` snapshot through
`win32_reevaluate_thread_window_backdrops`, 2 again after re-enabling, 3 for
`blur`, and 1 for `none`.

WSLg (Weston, kernel 6.18.33.2-microsoft-standard-WSL2) advertises neither
`ext_background_effect_manager_v1` nor `org_kde_kwin_blur_manager` (registry
listed with a throwaway libwayland client: wl_compositor, wl_subcompositor,
wl_shm, wl_seat, wl_output, wl_data_device_manager, xdg_wm_base,
zxdg_shell_v6, wl_shell, wp_viewporter, wp_presentation, text-input v1,
input-method v1, pointer constraints, relative pointer, xdg-output,
input timestamps, weston_rdprail_shell, weston_screenshooter). Wayland
backdrops therefore report `supported = false` there, the surface stays
opaque, and `transparency_effects_enabled` is false.

## Observed on screen (2026-11-17)

`example_client_title_bar` with `CGPUI_EXAMPLE_BACKDROP=1`, 144 DPI, dark
mode, transparency effects on, captured from the screen (PNG). Before the
erase fix every transparent client pixel was rgb(255,255,255) while the
painted caption row was exact. After it, the transparent area shows the
dark Mica material tinted by the desktop wallpaper: rgb(45..46,26,28..30)
near the left edge shifting to rgb(35..38,27..29,37..42) near the right,
varying smoothly. Painted pixels stay exact over the material: caption row
rgb(32,34,38) across all 1350 columns in rows 2..47 (rows 0..1 are
covered by the Windows 11 window border), command box
rgb(52,55,61) at x 495..854. DWM did not draw its own caption buttons:
every pixel of the caption row from client x 989 to 1338, rows 2..47, is
rgb(32,34,38) (the app paints its buttons in the caption color until
hovered), although `DWMWA_CAPTION_BUTTON_BOUNDS` reports a button area.

## Tests

- `vulkan_surface_composition_test`, `vulkan_transparent_composite_alpha_test`:
  composite alpha choice and report from synthetic masks (including the
  masks above).
- `win32_window_backdrop_test`: decision table with synthetic builds and
  settings, then a real window with DWM read-back and the synthetic
  appearance path.
- `vulkan_device_preference_test`: the device preference rule with the
  masks above.
- `win32_vulkan_backdrop_test`: real window and Vulkan renderer; the chosen
  device's composition and the applied backdrop must agree.
- `dx12_ordinary_backdrop_menu_test`: bounded initial system-title-bar DX12 alpha,
  actual Mica when OS/policy permits, exact native HMENU command-loop pixels,
  and `none` restoration with new GPU/desktop pixels. On builds below 26100,
  transparent creation must return `unsupported_platform` before positive cases
  are explicitly skipped; newer builds never skip a failed alpha initialization.
- `dx12_client_alpha_lifetime_test`: no-menu client-title-bar initial alpha,
  Mica application or explicit policy refusal, unchanged native topology, and
  `none` restoration with new title-row/body GPU and desktop pixels. Initial
  geometry/pixels and two idle-device removals are covered separately by
  `dx12_client_initial_alpha_test` and `dx12_client_alpha_recovery_test`.
- `window_runtime_backdrop_test`: the runtime helper with a fake window and
  renderer (switch to transparent once, inherit treated as opaque,
  unsupported backend leaves the surface alone, clear color).
- `window_runtime_transparent_clear_test`,
  `window_runtime_dynamic_backdrop_clear_test`: public runtime root/secondary
  creation and redraw through recording frames; explicit intent vs actual
  composition, live capability changes, and `Window::set_backdrop` removal,
  reapplication and material refusal. These are runtime policy tests, not
  desktop transparency evidence.
- `wayland_window_backdrop_test` (Linux): decision table; the test
  compositor without blur globals (unsupported, transparency off); the test
  compositor with a fake `ext_background_effect_manager_v1` global
  (`tests/platform/wayland_test_background_effect.cpp`, installed through
  `WaylandTestCompositor::add_global_installer`), checking transparency on,
  `applied = blur`, and one effect object, one region, one clear on the
  server.
- `win32_backdrop_erase_test`: real window; without a backdrop
  `WM_ERASEBKGND` paints nothing and the class brush is null; after
  `apply_window_backdrop(mica, true)` the redirection surface reads back
  black and a white repaint is erased to black (skipped with the reason
  when the frame cannot be extended).
- `window_backdrop_structure_test`, `backdrop_erase_structure_test` and
  three header-cleanliness tests.

## Not verified

- Mica was observed on one machine only (dark mode, NVIDIA device). Mica
  Alt and Acrylic were not looked at, nor the title-bar-only result on the
  Intel device.
- Resizing a Mica window was not watched for white or black flashes in
  areas the swapchain has not presented yet.
- Toggling transparency effects in Windows Settings has not been tried; the
  re-evaluation was driven with synthetic snapshots.
- KDE blur has no test (no fake global); ext blur was tested only against
  the fake global, not a real compositor (KWin, Mutter, or others).
- Visual quality of premultiplied text and rounded rects over a material is
  unchecked (UNORM swapchain, blending in sRGB-encoded space).
