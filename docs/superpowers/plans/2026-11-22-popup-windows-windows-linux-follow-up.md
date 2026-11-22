# Popup Windows: Windows and Linux Follow-up

Date: 2026-11-22
Status: in progress (see [Progress](#progress)). Covers popup windows and
native context menus. macOS is done; Win32 popup windows are done
(2026-11-22, DirectComposition presentation, see Progress); Wayland and X11
remain. See `docs/superpowers/plans/2026-11-22-native-popup-windows-plan.md`
(Progress) and `docs/workbench-widgets.md` (Popup windows).

Wayland and X11 report `PlatformApplication::popup_windows_supported()
== false`, so `Tooltip` and `PopoverMenu` use the in-window overlay there;
Win32 reports `true` when its DirectComposition probe passes.
The overlay stays as the fallback on every platform. Nothing above the
platform layer needs to change: a backend becomes popup-capable by
implementing the contract below and returning `true`.

## Contract a backend implements

1. `PlatformApplication::popup_windows_supported()` returns `true`.
2. `create_child_window(descriptor, parent, callback)` with
   `descriptor.kind == WindowKind::popup` creates a window that:
   - has no frame or title bar, is not in the taskbar, Alt+Tab or the
     window list, and never takes keyboard focus or activates the app;
   - is owned by `parent` (moves, minimizes and closes with it) and stays
     above it;
   - has a transparent, alpha-composited surface: the runtime creates its
     renderer with `transparent_background = true` and clears to (0,0,0,0);
     the widgets draw their own rounded corners and shadow;
   - is shown without activation, at `descriptor.popup` when it is set;
   - ignores pointer input (it passes through) when
     `descriptor.popup_options.pointer_transparent` is set (tooltips);
     otherwise it sends pointer events in its own client coordinates
     through `callback`, like any window;
   - is rejected when `create_window` (no parent) gets a popup kind.
3. `PlatformWindow::place_popup(WindowPopupPlacement)` on such a window
   moves it, at its current size, to `side` of `anchor` (the parent's
   logical client coordinates) with `gap`: below / above align left edges,
   right / left align top edges; flip to the other side when the preferred
   side leaves the work area of the monitor holding the anchor and the other
   side has more room; then shift along each axis to stay on that work
   area. It returns the final bounds in the parent's client coordinates
   and whether it flipped or shifted. Non-popup windows return
   `supported = false`.

Keyboard input never goes to popups: the runtime forwards the root window's
key and text events to a popup opened with `takes_keyboard`, and the menu
widget handles keys through the root view anyway. Pointer input in a menu
popup is handed back to the root window (`forward_pointer_input_to_root`),
so outside clicks, hit testing and activation are shared code.

The macOS implementation is the reference: `platform_popup_frame`
(shared placement math, y-down, `src/platform/platform_popup_placement.cpp`,
with a unit table in `platform_popup_placement_test`),
`macos_make_popup_panel` / `macos_attach_popup_panel`, and
`MacOSWindow::place_popup` (`src/platform/macos/macos_popup_window.mm`).

## Win32

- Window: `CreateWindowExW` with extended style `WS_EX_NOACTIVATE |
  WS_EX_TOOLWINDOW`, style `WS_POPUP`, and the parent `HWND` as owner
  (owned windows stay above their owner, so `WS_EX_TOPMOST` is not
  needed). Show with
  `ShowWindow(SW_SHOWNOACTIVATE)` and move with `SetWindowPos(...,
  SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER)`.
- `WM_MOUSEACTIVATE` returns `MA_NOACTIVATE`; `WM_NCHITTEST` returns
  `HTTRANSPARENT` when `pointer_transparent` (or add `WS_EX_TRANSPARENT |
  WS_EX_LAYERED`). Implemented with the layered transparent styles:
  `HTTRANSPARENT` hands the pointer to same-thread windows only (see
  Progress).
- Placement: anchor to screen with `ClientToScreen` after converting
  logical to physical pixels (`WindowState::scale`, as the IME placement
  does); work area from `MonitorFromPoint(..., MONITOR_DEFAULTTONEAREST)` +
  `GetMonitorInfoW(rcWork)`, then `platform_popup_frame` (shared, see
  Progress).
- Transparency: this is the hard part. The Vulkan swapchain on the Intel
  GPU of the test machine has no `PRE_MULTIPLIED` composite alpha
  (`docs/window-backdrop.md`), and `WS_EX_LAYERED` is not combined with
  the Vulkan surface today. Options: prefer the premultiplied-capable GPU
  for popups as for backdrops (`vulkan_device_preference.cpp`), or a
  DirectComposition path (see the DX11/DX12 discussion). Without
  transparency the shadow margin would show as a black frame, so keep
  `popup_windows_supported()` false until it composites. Resolved with the
  DirectComposition path below.
- Per-monitor DPI: a popup on another monitor gets that monitor's scale
  (`WM_DPICHANGED` on the popup); the runtime resizes its renderer from
  `WindowResized`, which already works for child windows.
- Tests: extend `win32_*` platform tests with the macOS cases (style bits,
  not activated, owner, placement and flip at a monitor edge,
  pointer-transparent hit test).

### Win32 design: DirectComposition presentation (stage B, for review)

Spike (2026-11-22, `D:\WSL\spike\spike.cpp`, outside the repo): Vulkan
cleared a D3D11 shared texture to premultiplied `128, 0, 0, 128`, D3D11
copied it into a premultiplied composition swapchain shown with DComp on a
`WS_EX_NOREDIRECTIONBITMAP` popup over a green window, and the screen read
back `128, 127, 0` (red over green) on the NVIDIA RTX 5070 Ti and on the
Intel GPU (`VK_LOADER_VENDOR_ID_FILTER` selected each).

Presentation path of a popup (per frame, one image in flight):
1. Vulkan renders into a `VkImage` imported from a D3D11 texture
   (`D3D11_RESOURCE_MISC_SHARED_NTHANDLE | SHARED_KEYEDMUTEX`,
   `B8G8R8A8_UNORM`) with `VK_KHR_external_memory_win32`
   (`D3D11_TEXTURE_BIT`, dedicated allocation). The submission acquires
   the keyed mutex with key 0 and releases key 1
   (`VK_KHR_win32_keyed_mutex`); two small pre-recorded command buffers
   around the frame's own command buffer move the image from and to
   `VK_QUEUE_FAMILY_EXTERNAL`.
2. D3D11, on the adapter whose LUID equals the Vulkan device's
   (`VkPhysicalDeviceIDProperties`, `IDXGIFactory4::EnumAdapterByLuid`),
   takes key 1, copies the texture into the back buffer of a
   `CreateSwapChainForComposition` swapchain (`DXGI_ALPHA_MODE_PREMULTIPLIED`,
   flip sequential, 2 buffers), releases key 0 and presents.
3. A DComp target on the popup HWND has one visual whose content is that
   swapchain (set and committed once; `ResizeBuffers` keeps it).

Why not the alternatives in the plan: DXGI swapchain buffers cannot be
shared, so Vulkan cannot render into them directly; a copy per popup
frame is small. The keyed mutex needs no extra objects and is reported by
all four devices here; a shared D3D11 fence and timeline semaphore would
need D3D11.4 and semaphore import for the same result. Preferring the
premultiplied-capable GPU for popups (as for backdrops) would leave
popups without transparency on Intel-only machines.

Seam in the renderer (smallest found; split in two by the amendment
below): the platform marks the popup's
surface, `Win32SurfaceHandle::composition = true` ("no redirection
surface; present through DirectComposition"). The Vulkan renderer then
creates no `VkSurfaceKHR`, takes the first Vulkan 1.1 device with
graphics and the two interop extensions, and builds its swapchain
resources from a `VulkanCompositionPresenter` (new files,
`src/renderer/vulkan/vulkan_composition*`) instead of `VkSwapchainKHR`:
the same render pass, pipelines, framebuffers, command reuse and pixel
capture over one imported image. Only three places branch on it: image
creation (`create_swapchain_resources`), submission (keyed mutex instead
of semaphores) and acquire/present in `present_frame`. The runtime and
`Renderer` API do not change. Normal windows keep `VkSwapchainKHR`
unchanged (no flag, same device selection and code path).

Platform side: popups are `WS_POPUP` with `WS_EX_NOACTIVATE |
WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP`, owned by the parent, shown
with `SW_SHOWNOACTIVATE`; a window property marks them for the window
procedure (`WM_MOUSEACTIVATE` -> `MA_NOACTIVATE`, `WM_NCHITTEST` ->
`HTTRANSPARENT` when pointer-transparent; Windows passes that hit to the
same-thread window below, the root). `place_popup` converts the anchor
with the parent's scale and `ClientToScreen`, places with
`platform_popup_frame` on the anchor monitor's work area, and returns the
bounds in the parent's logical client coordinates. Owned windows do not
move with their owner on Windows, so the owner's `WM_MOVE` moves its
popups by the same offset. `WM_DPICHANGED` on a popup resizes it to the
new monitor's scale through the existing path.

Fallback: `popup_windows_supported()` is true when a probe (a D3D11
hardware device and `DCompositionCreateDevice`, once per application)
succeeds. If a popup's renderer cannot start (no interop extensions, no
LUID match, a failed import), `open_popup` fails with that error and the
widgets keep the in-window overlay, as for any open failure.

Found while designing: `Win32Application::windows_` keeps raw pointers of
destroyed windows and `dispatch_wakeup` calls them; popups are destroyed
often (every tooltip), so destroyed windows leave the list first.

Not covered: sharing one Vulkan device across windows (each popup creates
its own instance, device, D3D11 device and DComp device; creation time is
measured in the pixel test), and popups on a monitor whose scale differs
from the parent's beyond `WM_DPICHANGED`.

### Win32 design amendment: presentation split for a DX12 backend

A DX12 renderer backend is planned on Windows next to Vulkan. Its popups
must reuse the DXGI and DirectComposition half of the path above without
any Vulkan code, so the presentation is split in two parts (this replaces
the single `VulkanCompositionPresenter` of the seam paragraph above):

1. A Windows-only composition module, `src/renderer/win32_composition/`,
   in its own package `packages/renderer-win32-composition` (Windows
   sources, an empty library elsewhere, like `renderer-vulkan`). It
   depends on `cgpui.core` only and links `d3d11`, `dxgi` and `dcomp`.
   `Win32CompositionPresenter::create(LUID, HWND, size)` owns:
   - the D3D11 device on the adapter with that LUID
     (`IDXGIFactory4::EnumAdapterByLuid`);
   - the premultiplied composition swapchain
     (`CreateSwapChainForComposition`, flip sequential, 2 buffers) and
     its `ResizeBuffers`;
   - the DComp device, the target on the HWND and the visual whose
     content is the swapchain, committed after the first present;
   - the shared texture (`B8G8R8A8_UNORM`, `SHARED_NTHANDLE |
     SHARED_KEYEDMUTEX`); `shared_texture_handle()` returns its NT handle,
     which a resize replaces;
   - `present()`: the D3D11 side acquires the keyed mutex with key 1,
     copies the texture into the back buffer, releases key 0 and
     presents.
   `win32_composition_available()` is the probe behind
   `popup_windows_supported()`: a hardware D3D11 device and a DComp
   device, once per process. The Win32 platform calls it, which is why
   the module depends on nothing but core.
2. The Vulkan side, `src/renderer/vulkan/vulkan_composition_*`: device
   selection for composition surfaces (the first Vulkan 1.1 device with
   graphics, `VK_KHR_external_memory_win32`, `VK_KHR_win32_keyed_mutex`,
   a valid LUID and an importable D3D11 texture format), and a
   `VulkanCompositionPresenter` that imports the module's handle as a
   `VkImage` (external memory, dedicated allocation), records the
   ownership transfers from and to `VK_QUEUE_FAMILY_EXTERNAL`, submits
   the frame with the keyed mutex (acquire key 0, release key 1,
   `VK_KHR_win32_keyed_mutex`) and then calls the module's `present()`.
   The renderer uses it instead of `VkSwapchainKHR` for surfaces marked
   `Win32SurfaceHandle::composition`, that is for popups only.

The DX12 plan (`2026-11-22-dx12-renderer-backend-plan.md`, section 4.2)
replaced the idea that a DX12 backend opens the same handle and calls the
same `present()`. D3D12 has no keyed mutex, so DX12 creates its own
composition swapchain on its command queue and reuses only the DComp
device, target and visual. Those still sit in `Win32CompositionObjects`
next to the D3D11 objects; DX12 phase 5 moves them into a type of their
own. Normal windows keep `VkSwapchainKHR`, unchanged.

## Wayland

- Window: `xdg_surface.get_popup(parent xdg_surface, xdg_positioner)`.
  The positioner does the placement: `set_size`, `set_anchor_rect` (the
  anchor in the parent's surface coordinates), `set_anchor` / `set_gravity`
  from `side`, `set_offset` for `gap`, and `set_constraint_adjustment`
  (`flip_x | flip_y | slide_x | slide_y`) for flip and shift. The final
  position arrives in `xdg_popup.configure`; report it from `place_popup`
  after the configure round trip (or return the requested bounds and send
  `WindowMoved` when the configure differs).
- Moving: `xdg_popup.reposition` (version 3) with a new positioner.
- Grab: menus need `xdg_popup.grab(seat, serial)` with the serial of the
  input that opened them (the press, or the key for keyboard-opened menus).
  The compositor then dismisses the popup on outside clicks with
  `popup_done`; turn that into a close of the widget (send a
  `WindowCloseRequested` on the popup record, or a focus loss to the root).
  Tooltips use no grab.
- Pointer transparency: an empty input region
  (`wl_surface.set_input_region` with an empty `wl_region`).
- Transparency: the WSLg Vulkan surfaces offer `PRE_MULTIPLIED`
  (`docs/window-backdrop.md`), so a transparent swapchain should work.
- Keyboard: popups with a grab may receive keyboard focus from the
  compositor; route those events to the root window like the runtime does
  for `takes_keyboard`, or ignore them.
- Tests: the private test compositor (`tests/platform/wayland_test_compositor.cpp`)
  needs `xdg_popup` and positioner support to check requests and configure.

## X11

- Window: override-redirect top-level (`override_redirect = 1`,
  `_NET_WM_WINDOW_TYPE_POPUP_MENU` / `_TOOLTIP`), transient for the parent,
  ARGB visual for transparency.
- Placement: translate the anchor with `xcb_translate_coordinates` to the
  root window; work area from `_NET_WORKAREA` or RandR monitor geometry.
- No compositor grab: outside clicks need a pointer grab while a menu is
  open, or the root window's focus-out (already closes menus).
- Pointer transparency: an empty input shape (XFixes / Shape extension).

## Native context menus (`show_context_menu`)

Separate from popup windows: `PlatformWindow::show_context_menu(NativeMenuModel,
Point)` shows the system's own menu, and `PopoverMenuOptions::prefer_native_menu`
makes `PopoverMenu::open_at` try it first. The base implementation returns
`supported = false`, and the widget then opens its own menu (popup window
or overlay). Contract, as on macOS (`src/platform/macos/macos_context_menu.mm`,
`macos_context_menu_test`):

- `position` is in the window's logical client pixels.
- The call returns when the menu closes. `shown` is true when it was shown;
  `action_name` holds the chosen command, if any.
- A chosen command is also sent as `NativeMenuCommand` with source
  `NativeMenuCommandSource::context_menu`, one event-loop turn later, so it
  does not re-enter the click handler that opened the menu. Guard it
  against the window being destroyed meanwhile (macOS uses a lifetime
  token, `MacOSWindow::lifetime_`).
- Item `enabled` and `checked` come from the model; separators and submenus
  as in the model. An empty model returns `shown = false` with a reason.
- Accelerators are not shown: `PopoverMenu` converts `MenuItem::accelerator`
  (a display string) to no `NativeMenuAccelerator`.

Win32 (done 2026-11-22, see Progress):
- Build the menu with `CreatePopupMenu` and the existing item code in
  `src/platform/win32/win32_native_menu_tree.cpp` (it builds the menu bar
  with `CreateMenu` today; add a popup root). Map command ids to action
  names as `win32_native_menu_command.cpp` does.
- Show it with `TrackPopupMenuEx(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON |
  TPM_NONOTIFY, x, y, hwnd, nullptr)`, the point converted with the window
  scale and `ClientToScreen`, as the title bar's `show_window_menu` does
  (`win32_window_title_bar_actions.cpp`). `TPM_RETURNCMD` returns the chosen
  id instead of posting `WM_COMMAND`, so the menu command is not dispatched
  twice; post the `NativeMenuCommand` afterwards (for example with a private
  `PostMessageW` the window procedure turns into the event).
- Call `SetForegroundWindow(hwnd)` before tracking and post `WM_NULL`
  after it (documented `TrackPopupMenu` requirement for a menu that closes
  on outside clicks).
- Tests: a `win32_context_menu_test` that cancels the menu from a timer
  (`EndMenu`) and chooses an item through `WM_MENUSELECT` / keyboard input,
  like `macos_context_menu_test` does with `NSMenu` tracking.

Wayland and X11: no system context menu exists without a toolkit (GTK or
Qt menus would add a dependency the project does not have; dbusmenu and
the KDE/Unity app menus cover menu bars only). Keep `supported = false`;
`prefer_native_menu` then falls back to the widget's own menu, which uses
popup windows once the backend has them.

## Shared work before the first backend

- Move `macos_popup_frame` into `src/platform/platform_popup_placement.cpp`
  as a platform-neutral function and keep the unit table as a
  cross-platform test.
- Keep `workbench_popup_windows_test` and `window_runtime_popup_test` (fake
  platforms) as the widget and runtime contract; each backend adds only
  platform tests.

## Progress

Shared placement (2026-11-22): `macos_popup_frame` moved to
`platform_popup_frame` (`src/platform/platform_popup_placement.cpp`,
internal header `platform_popup_placement_internal.hpp`, compiled into
`packages/platform` on every OS); `MacOSWindow::place_popup` calls it.
Behaviour is unchanged. `platform_popup_placement_test` (tests/platform,
all platforms) holds the former macOS table plus above, left, no room on
either side, a work area off the origin and a popup larger than the work
area; `popup_placement_structure_test` keeps the function neutral and
shared. Not built on macOS in this change.

Win32 native context menu (2026-11-22): `Win32Window::show_context_menu`
(`src/platform/win32/win32_window_context_menu.cpp`) builds the model with
`Win32NativeMenuTree::build_popup` (the menu bar item code under a
`CreatePopupMenu` root) and a `Win32NativeMenuCommandMap` of its own, so
ids never mix with the installed menu bar. The point goes from logical
client pixels to physical with `WindowState::scale`, then `ClientToScreen`;
`SetForegroundWindow` before and `WM_NULL` after `TrackPopupMenuEx(
TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY)`. A choice is queued on the
window and announced with a private `WM_APP + 2` message
(`win32_context_menu_command_message`); the window procedure turns it into
`NativeMenuCommand{source = context_menu}` on the next loop turn. A window
destroyed during or after the menu sends nothing (`IsWindow` after
tracking; the destructor clears `GWLP_USERDATA` before the message
arrives). `result.shown` is false only for an empty model, a closed window,
a failed build, or `ERROR_POPUP_ALREADY_ACTIVE`. `win32_context_menu_test`
drives the modal loop from a thread timer: it checks the open `HMENU`
(`MN_GETHMENU`: separator, checked, disabled, submenu), cancels with
`EndMenu`, chooses with posted arrow and Enter keys (a top item and a
submenu item; the command arrives only after the call returns), the empty
model, and a window destroyed before delivery. Wayland and X11 keep
`supported = false`. Not checked on screen (example_workbench with
`CGPUI_WORKBENCH_NATIVE_MENU=1`).

Win32 popup windows (2026-11-22, stage B with the DX12 amendment):
- Composition module (`packages/renderer-win32-composition`,
  `src/renderer/win32_composition`): `Win32CompositionPresenter` and
  `win32_composition_available()` as in the amendment. It holds no Vulkan
  code (`win32_composition_structure_test`). `win32_composition_presenter_test`
  renders with a second D3D11 device in place of a backend, on every
  hardware adapter, and reads the screen back: 50% red over green reads
  128, 127, 0 on Intel and NVIDIA; after a resize to 32 x 32, 50% blue
  reads 0, 127, 128 and the uncovered part of the popup shows the green
  window (0, 255, 0); a present without a rendered frame fails after one
  second instead of hanging.
- Vulkan side (`src/renderer/vulkan/vulkan_composition_*`): keyed on
  `Win32SurfaceHandle::composition`, not on the window kind. The device is
  the first enumerated Vulkan 1.1 device with graphics, the two interop
  extensions, a LUID and an importable D3D11 texture format, with no
  premultiplied-alpha preference (Intel(R) Graphics here); swapchain
  windows keep their selection. The branches are `create_surface`,
  `select_physical_device`, `create_device`, `create_swapchain_resources`,
  `submit_frame` and the acquire and present in `present_frame` (stage B
  counted three). A composition frame ends its render pass in
  `COLOR_ATTACHMENT_OPTIMAL`, because `PRESENT_SRC_KHR` is for presentable
  images only, so the pixel capture command carries the layout it
  restores.
- Platform (`src/platform/win32/win32_popup_window*.cpp`,
  `win32_popup_placement.cpp`, `win32_window_proc_popup.cpp`): a window
  class of its own whose extra bytes keep the popup state (the owner
  offset), so `Win32Window` only declares `place_popup`;
  `native_surface()` sets `composition` for popups only. Popups never
  enter `Win32Application::windows_` (no menu bar, no wakeups), so
  destroyed tooltips leave no dangling pointer there; destroyed normal
  child windows still do (unchanged).
- Pointer transparency: `HTTRANSPARENT` hands the pointer to windows of
  the same thread only. An experiment outside the repo put a window of
  another thread under a DComp popup and moved the mouse over it with
  `SendInput`: with `HTTRANSPARENT` neither window received a
  `WM_MOUSEMOVE` (the popup only got the hit tests); with
  `WS_EX_LAYERED | WS_EX_TRANSPARENT` the window below received 11 and the
  popup none (6 and none for a same-thread window below), and the
  popup's DComp content still read 128, 127, 0. Pointer-transparent
  popups use those two styles (`SetLayeredWindowAttributes` 255, alpha
  unchanged); other popups and windows do not get `WS_EX_LAYERED`.
- Tests: `win32_popup_window_test` (scale 1.5 here: placement below an
  anchor at 10, 34, 120 x 80 in parent coordinates, the popup moving 30,
  20 with its parent, a flip above at the work area's bottom edge, and
  `WindowFromPoint` at a pointer-transparent popup returning the parent);
  `vulkan_composition_pixel_test` and `win32_popup_pixel_test`, once per
  GPU vendor with `VK_LOADER_VENDOR_ID_FILTER`: a 50% red rect over a
  transparent clear is captured as 128, 0, 0, 128 on Intel and
  127, 0, 0, 127 on NVIDIA, the screen reads 128, 127, 0 (127, 128, 0 on
  NVIDIA) over green, and the cleared half shows the green window.
- Creation cost per popup (each opens its own Vulkan instance and device,
  D3D11 device and DComp device), over five test runs: a composition
  renderer took 65 to 132 ms on Intel and 99 to 485 ms on NVIDIA (317 to
  485 ms when it was the process's first renderer on NVIDIA, 99 to 170 ms
  after the parent's); the module alone 45 to 106 ms on Intel and 180 to
  199 ms on NVIDIA. Sharing devices across popups is still not covered.
- Not verified: the on-screen look of `example_workbench` menus and
  tooltips in popups, multi-monitor placement (one 2560 x 1600 display at
  150% here), and a popup moving to a monitor of another scale
  (`WM_DPICHANGED`).

## Known limits on macOS (for parity checks)

- Menus in popups draw an opaque surface: in-app backdrop blur has nothing
  of the app behind a separate window. An `NSVisualEffectView` backdrop on
  the popup would give a system blur; not done.
- The overlay node of an open menu still covers the root window, so
  outside clicks on the root close the menu through shared code.
- Popups are children of the root window only.
