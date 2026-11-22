# Native Popup Windows (C9 step 2)

Date: 2026-11-22
Status: approved 2026-11-22; steps 1 and 2 done (see [Progress](#progress)). The in-window overlay stays as the fallback where popups are unsupported or fail.

## Goal

Tooltips, context menus and popovers that can extend outside the window,
keep the app's own drawing (theme, shadow, C6 blur) and keep the public
widget API (`PopoverMenu`, `Tooltip`). Step 1 (native NSMenu,
`PopoverMenuOptions::prefer_native_menu`) stays as the system-look option.

## What exists

- The runtime already runs several native windows: `open_window` on a
  running runtime creates a child window (`parent_runtime_id`) with its own
  view, renderer, framebuffer, input state and event routing
  (`WindowRuntimeRecord`).
- Child windows are owned by their parent: macOS `addChildWindow:ordered:`,
  Win32 an owner `HWND` in `CreateWindowExW`, Wayland
  `xdg_toplevel_set_parent`. They are still normal framed windows.
- `WindowDescriptor::position` is in physical virtual-screen coordinates.

## Design

1. Platform: `WindowDescriptor::kind` (`normal`, `popup`). A popup window
   has no frame, never takes key focus, is not in the window list or the
   Dock/taskbar, follows its parent (moves, hides and closes with it), and
   is placed relative to the parent's client area:
   - macOS: non-activating `NSPanel` (`NSWindowStyleMaskNonactivatingPanel`,
     `becomesKeyOnlyIfNeeded`, `hidesOnDeactivate`), child of the parent,
     level `NSPopUpMenuWindowLevel`; transparent Metal layer for shadow and
     rounded corners.
   - Win32: `WS_POPUP` with `WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW`, owned by
     the parent; `WM_MOUSEACTIVATE` returns `MA_NOACTIVATE`.
   - Wayland: `xdg_popup` with an `xdg_positioner` on the parent surface and
     a grab for menus (compositors close it on outside clicks).
   - X11: override-redirect window. Backends without support report it,
     and widgets keep the in-window overlay.
2. Placement: a popup is requested with an anchor rect in the parent's
   logical client coordinates plus a side, like `anchored.hpp`. The backend
   converts to screen coordinates and flips or shifts against the work area
   of the screen that holds the anchor (instead of the window bounds).
3. Runtime: `WindowRuntime::open_popup(anchor, side, size, view)` opens a
   popup window for a view and returns its id; `close_popup(id)`. Input in
   the popup routes to its view. Keyboard input stays with the parent
   (popups are non-activating), so the parent forwards key events to the
   open popup's view first (menu navigation, Escape).
4. Dismissal: outside clicks close menus. macOS: a local event monitor on
   the parent plus `NSWindowDidResignKeyNotification`; Win32: the owner's
   `WM_ACTIVATE` / mouse hooks; Wayland: the popup grab's `popup_done`.
5. Widgets: `PopoverMenu` and `Tooltip` get a `native_window` option. When
   the runtime can open popups, the widget lays out and paints into the
   popup's view (the same layout, paint and accessibility code), and the
   in-window overlay node is not added. Submenus open as further popups.
6. Accessibility: the popup's tree is published to its own window; on macOS
   the panel is an accessibility child of the parent window.

## Order

1. macOS platform popup kind and placement, with a platform test.
2. Runtime popups (open, close, input and key forwarding, dismissal), with
   TestApp coverage.
3. `Tooltip`, then `PopoverMenu` (with submenus) on popups; workbench
   switch; screen check.
4. Win32, then Wayland and X11, each with platform tests; until then they
   keep the in-window overlay.

## Risks and open points

- Keyboard forwarding from a non-activating popup is new routing; the
  menu's focus model (focus restore on close) has to keep working.
- A frame per popup window means another renderer per popup; popups are
  short-lived, but Metal/Vulkan device creation must stay cheap (share the
  device per process if it shows in timings).
- Wayland `xdg_popup` requires a recent input serial for grabs; tooltips
  use no grab.
- Multi-monitor placement and per-monitor scale of a popup on another
  display than its parent.
- Estimated size: comparable to C3-C6 together. Steps 1-3 are macOS-only
  and usable on their own; step 4 brings the other platforms.

## Progress

Step 1 (macOS platform, 2026-11-22):
- `WindowDescriptor::kind` / `popup` (`include/cgpui/core/window_popup.hpp`),
  `PlatformWindow::place_popup` returning `PlatformPopupPlacementResult`
  (`platform_window_popup.hpp`), `PlatformApplication::popup_windows_supported`
  (false except on macOS). Win32, Wayland and X11 do not create popup kinds
  yet and the runtime must not ask them to.
- macOS (`src/platform/macos/macos_popup_window.mm`): borderless
  non-activating `NSPanel`, transparent, no system shadow, transient in
  Spaces and Mission Control. `addChildWindow:` resets the child's level and
  hides-on-deactivate, so both are applied after it
  (`NSPopUpMenuWindowLevel`, YES). Shown with `orderFront:` only; the app is
  not activated. A popup without a parent is rejected.
- Placement converts the anchor from the parent's flipped CGPUI view to
  screen coordinates, places in the visible frame of the screen holding the
  anchor's center (flip, then shift; `macos_popup_frame`), and reports the
  bounds back in the parent's client coordinates.
- Content views track the mouse while the app is active
  (`NSTrackingActiveInActiveApp`, was key window only), so non-key popups
  get hover.
- `macos_popup_window_test`: placement table (below, flip above, flip
  left with a shift, shift left), panel properties, placement in parent
  coordinates, and the popup moving with its parent.

Step 2 (runtime, 2026-11-22):
- `include/cgpui/ui/runtime_popups.hpp`: `popup_windows_supported`,
  `open_popup(runtime, placement, size, view, takes_keyboard)`,
  `place_popup`, `request_popup_redraw`, `close_popup`. Free functions,
  because `WindowRuntime`'s header is capped at 263 lines; private access
  goes through `RuntimePopups` (a friend, `src/ui/runtime_popups_internal.hpp`).
- A popup is a child record of the root window (`try_open_window` with
  `WindowOptions::popup`), with a transparent renderer and a transparent
  clear. Popups are never key windows, so the root window's `KeyboardKey`
  and `TextInput` go to the popup opened with `takes_keyboard` until it
  closes. `open_popup` fails with `unsupported_platform` where the backend
  has no popups; widgets then keep the overlay.
- `window_runtime_popup_test` (fake platform, all platforms): unsupported
  backend, popup descriptor and transparent renderer, keyboard forwarding,
  placement and redraw forwarding, closing (keys return to the root).
