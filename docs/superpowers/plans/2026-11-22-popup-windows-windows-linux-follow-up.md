# Popup Windows: Windows and Linux Follow-up

Date: 2026-11-22
Status: in progress (see [Progress](#progress)). Covers popup windows and
native context menus. macOS is done; see
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
  WS_EX_LAYERED`).
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

Win32 (recommended; small):
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

## Known limits on macOS (for parity checks)

- Menus in popups draw an opaque surface: in-app backdrop blur has nothing
  of the app behind a separate window. An `NSVisualEffectView` backdrop on
  the popup would give a system blur; not done.
- The overlay node of an open menu still covers the root window, so
  outside clicks on the root close the menu through shared code.
- Popups are children of the root window only.
