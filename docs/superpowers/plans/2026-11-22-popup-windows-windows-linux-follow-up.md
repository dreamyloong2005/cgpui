# Popup Windows: Windows and Linux Follow-up

Date: 2026-11-22
Status: to do. macOS is done; see
`docs/superpowers/plans/2026-11-22-native-popup-windows-plan.md` (Progress)
and `docs/workbench-widgets.md` (Popup windows).

Win32, Wayland and X11 report `PlatformApplication::popup_windows_supported()
== false`, so `Tooltip` and `PopoverMenu` use the in-window overlay there.
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

The macOS implementation is the reference: `macos_popup_frame` (placement
math, y-down, with a unit table in `macos_popup_window_test`),
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
  WS_EX_LAYERED`).
- Placement: anchor to screen with `ClientToScreen` after converting
  logical to physical pixels (`WindowState::scale`, as the IME placement
  does); work area from `MonitorFromPoint(..., MONITOR_DEFAULTTONEAREST)` +
  `GetMonitorInfoW(rcWork)`. Port `macos_popup_frame` to a shared,
  platform-neutral function (it is pure math) instead of copying it.
- Transparency: this is the hard part. The Vulkan swapchain on the Intel
  GPU of the test machine has no `PRE_MULTIPLIED` composite alpha
  (`docs/window-backdrop.md`), and `WS_EX_LAYERED` is not combined with
  the Vulkan surface today. Options: prefer the premultiplied-capable GPU
  for popups as for backdrops (`vulkan_device_preference.cpp`), or a
  DirectComposition path (see the DX11/DX12 discussion). Without
  transparency the shadow margin would show as a black frame, so keep
  `popup_windows_supported()` false until it composites.
- Per-monitor DPI: a popup on another monitor gets that monitor's scale
  (`WM_DPICHANGED` on the popup); the runtime resizes its renderer from
  `WindowResized`, which already works for child windows.
- Tests: extend `win32_*` platform tests with the macOS cases (style bits,
  not activated, owner, placement and flip at a monitor edge,
  pointer-transparent hit test).

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

## Shared work before the first backend

- Move `macos_popup_frame` into `src/platform/platform_popup_placement.cpp`
  as a platform-neutral function and keep the unit table as a
  cross-platform test.
- Keep `workbench_popup_windows_test` and `window_runtime_popup_test` (fake
  platforms) as the widget and runtime contract; each backend adds only
  platform tests.

## Known limits on macOS (for parity checks)

- Menus in popups draw an opaque surface: in-app backdrop blur has nothing
  of the app behind a separate window. An `NSVisualEffectView` backdrop on
  the popup would give a system blur; not done.
- The overlay node of an open menu still covers the root window, so
  outside clicks on the root close the menu through shared code.
- Popups are children of the root window only.
