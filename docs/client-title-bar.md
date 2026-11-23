# Client title bar

A window can let the app draw its own title bar row (project name, command
box, caption buttons) inside the client area while the platform keeps the
native window behaviors: move by dragging the caption, resize from every
edge, double-click to maximize, the system/window menu, Aero Snap and, on
Windows 11, the Snap Layouts flyout over the maximize button.

## API

`include/cgpui/core/window_title_bar.hpp`:

```cpp
enum class WindowTitleBarMode { system, client };
enum class WindowControlArea {
  caption, minimize_button, maximize_button, close_button, client };
struct WindowControlRegion { WindowControlArea area; Rect bounds; };  // logical px
enum class WindowResizeEdge {
  top, bottom, left, right, top_left, top_right, bottom_left, bottom_right };
```

`WindowControlArea` mirrors GPUI's `WindowControlArea` (`Drag`, `Min`, `Max`,
`Close`). `client` is the CGPUI addition: an interactive control inside the
caption row (a command box) that must not drag.

`WindowChromeOptions::title_bar` (default `system`) selects the mode;
`WindowOptions::title_bar(WindowTitleBarMode)` sets it from the builder.
`apply_window_chrome` switches it at runtime, except while a Windows ordinary
DX12 alpha lease fixes the initial mode (see below).

`include/cgpui/platform/platform_window_title_bar.hpp`:

```cpp
struct WindowFrameInsets { float top, right, bottom, left; };
struct PlatformWindowFrameMetrics {
  bool supported; bool client_title_bar; std::string backend;
  float caption_height;          // system title bar height, 0 = no suggestion
  Size caption_button_size;      // system caption button size
  float resize_border;           // resize band used for hit tests
  WindowFrameInsets client_resize_border;  // part of the band inside the client area
  float caption_buttons_reserved_width;    // native buttons left in the row (0 here)
  float caption_buttons_leading_reserved_width;  // native buttons at the left (macOS)
  std::string mechanism, reason;
};
enum class PlatformWindowAction {
  minimize, toggle_maximize, close, show_window_menu, begin_move, begin_resize };
struct PlatformWindowActionRequest {
  PlatformWindowAction action; Point position; WindowResizeEdge edge; };
struct PlatformWindowActionResult { bool supported, performed; std::string reason; };
```

All sizes are logical pixels at the window's current scale.

| Layer | Entry points |
| --- | --- |
| `PlatformWindow` | `set_window_control_regions(std::vector<WindowControlRegion>)`, `frame_metrics() const`, `perform_window_action(PlatformWindowActionRequest)` |
| `Window` (app facade) | `frame_metrics() const`, `perform_window_action(request) const` |
| `PaintList` | `add_window_control_region(WindowControlArea, Rect)`, `window_control_regions() const` |
| `ElementBuilder` | `window_control_area(WindowControlArea)` (wraps the element in `WindowControlAreaElement`, `include/cgpui/ui/element_window_control_nodes.hpp`) |
| `render_view` | overload with `std::vector<WindowControlRegion>* window_control_regions` |

The base `PlatformWindow::perform_window_action` implements `minimize` and
`toggle_maximize` with the existing `request_display_state` and `close` with
`request_close`. `show_window_menu`, `begin_move` and `begin_resize` need
native support; the base returns `supported = false` with a reason, and so
does `frame_metrics()` (X11, test platforms).

### Declaring regions

Regions are collected during paint, like GPUI's window control hitboxes:

- A `View::paint` calls `paint_list.add_window_control_region(area, rect)`.
- An element built with `.window_control_area(area)` adds its painted bounds
  when it paints. Its layout, hit testing and events go to the wrapped
  element unchanged.

Bounds pass through the current paint transform and are clipped by the
current clip; empty regions are dropped. Regions are matched from last to
first, so a region painted later (a child, a control drawn over the caption)
wins. A point outside every region is ordinary client area.

Each frame the runtime sends the list to the window
(`runtime_publish_window_control_regions`, src/ui/runtime_window_title_bar.cpp):
the regions the frame painted. `render_view` paints `View::paint` first and
then the laid-out element tree the view returned from `render()` (root and
secondary windows), so view regions come first, then element regions, in
both title bar modes. Static element trees carry no paint data and add no
regions.

Caption buttons need no click handler: the platform minimizes, toggles
maximize or closes when the left button is released over the button region
it was pressed on. The app still receives the pointer events for hover and
pressed visuals. Do not also call the action from an `on_click`, or it runs
twice. `perform_window_action` is for other controls (a menu item, a custom
drag handle).

```cpp
auto window = app.open_window(cgpui::WindowOptions{}
    .title("Editor").title_bar(cgpui::WindowTitleBarMode::client));
const auto metrics = app.window(window.runtime_id)->frame_metrics();
const float row = metrics.caption_height > 0 ? metrics.caption_height : 32.0F;
// render(): h_flex().h(row)
//   .child(div().flex_1().window_control_area(WindowControlArea::caption))
//   .child(command_box().window_control_area(WindowControlArea::client))
//   .child(button().window_control_area(WindowControlArea::close_button)) ...
```

`examples/client_title_bar` paints a caption row, a command box and three
buttons from `View::paint`. It is built (`example_client_title_bar`, Windows
and Linux) and started with `CGPUI_EXIT_AFTER_FIRST_FRAME=1`; it is not a
test.

## Windows

Source: src/platform/win32/win32_window_title_bar.cpp (messages),
win32_window_title_bar_hit_test.cpp (metrics, hit test, client rect),
win32_window_title_bar_actions.cpp (`frame_metrics`, actions),
win32_window_proc_title_bar.cpp (window procedure hook).

- Style: client mode keeps `WS_CAPTION` even when `titlebar_visible` is
  false (`win32_window_style_for`), so snap, the system menu and the
  minimize/maximize animations stay. Fullscreen and `decorations = false`
  turn the client title bar handling off.
- `WM_NCCALCSIZE`: the client rect is the proposed window rect minus the
  frame on the left, right and bottom; the top keeps no non-client area, so
  the client starts at the window's top edge. A maximized window
  (`IsZoomed`) is also inset by the frame at the top, because Windows pushes
  a maximized window's frame off-screen on every edge. The frame comes from
  `AdjustWindowRectExForDpi` for the window's style at the tracked DPI.
- `WM_NCHITTEST`: unless maximized or non-resizable, points within the resize
  band of the window rect map to `HTLEFT`, `HTRIGHT`, `HTBOTTOM`, `HTTOP` and
  the four corners. The band is `SM_CXSIZEFRAME + SM_CXPADDEDBORDER` from
  `GetSystemMetricsForDpi` at the window's DPI. Left, right and bottom bands
  are the invisible non-client frame; the top band lies inside the client
  area (reported as `client_resize_border.top`). Then the regions, last
  first: `caption` → `HTCAPTION`, `minimize_button` → `HTMINBUTTON`,
  `maximize_button` → `HTMAXBUTTON` (what Windows 11 needs for the Snap
  Layouts flyout), `close_button` → `HTCLOSE`, `client` → `HTCLIENT`.
  Everything else is `HTCLIENT`. `HTCAPTION` gives native drag,
  double-click maximize, Aero Snap and the right-click system menu through
  `DefWindowProc`.
- Non-client mouse: over `HTCAPTION` and the button codes Windows sends
  `WM_NCMOUSEMOVE` instead of `WM_MOUSEMOVE`. It is forwarded to the app as
  `PointerMoved` (client logical coordinates) and still passed to
  `DefWindowProc`. `TrackMouseEvent(TME_LEAVE | TME_NONCLIENT)` produces
  `WM_NCMOUSELEAVE`, which becomes `PointerExited` when the cursor left the
  window rect. `WM_NCLBUTTONDOWN`, `WM_NCLBUTTONDBLCLK` and `WM_NCLBUTTONUP`
  over a button code become `PointerButton` events and are not passed to
  `DefWindowProc` (which would track and draw classic caption buttons). A
  release over the button the press started on posts `WM_SYSCOMMAND` with
  `SC_MINIMIZE`, `SC_MAXIMIZE` / `SC_RESTORE`, or `SC_CLOSE` (which arrives
  as `WM_CLOSE` through the close policy, source `window_manager`).
- DPI: the frame DPI is read with `GetDpiForWindow` when the chrome is
  applied, and set from `WM_DPICHANGED` before its `SetWindowPos`. In client
  mode that call also passes `SWP_FRAMECHANGED`, so the frame is recalculated
  even when the suggested rect keeps the same size.
- Actions: `begin_move` posts `WM_SYSCOMMAND(SC_MOVE + HTCAPTION)`,
  `begin_resize` posts `SC_SIZE + WMSZ_<edge>` (both after `ReleaseCapture`),
  `show_window_menu` runs `TrackPopupMenu` on `GetSystemMenu` with restore,
  move, size and maximize enabled for the current state, then posts the
  chosen command. Minimize, maximize and close use the base implementation.
- Metrics: `caption_height` is `-AdjustWindowRectExForDpi(WS_OVERLAPPEDWINDOW).top`
  (caption plus top frame of a native title bar), `caption_button_size` is
  `SM_CXSIZE` x `SM_CYSIZE`, and `resize_border` is the band above, all
  divided by the scale. `caption_buttons_reserved_width` is 0 because the app
  draws all buttons.
- Backdrop (C3): the DWM frame margins are unchanged. A backdrop over an
  alpha-composited surface still extends the frame with margins of -1; every
  other state keeps margins of 0. No top margin is set for the caption row:
  the app draws it, and DWM content under an opaque client area is covered
  anyway. The backdrop plan gained a `client_title_bar` input: with an opaque
  surface in client mode it reports `title_bar = none`, because the system
  title bar the material would be drawn on is gone. Changing the title bar
  mode re-applies a requested backdrop.

### Initial DX12 alpha (bounded)

On Windows build **26100+**, a CGPUI-owned ordinary window created with a visible,
decorated client title bar can request initial DX12 transparency. Use
`chrome.title_bar = WindowTitleBarMode::client` and initial
`chrome.transparent_background = true` (or a creation-time backdrop); the real
DWM alpha attribute must succeed. No chrome setter before renderer creation or
public composition/window-mode option is needed. This route requires **no native
application HMENU bar**. The system/window menu obtained through `GetSystemMenu`
is not that application menu and remains part of the native contract.

The existing `WS_CAPTION`, `WS_SYSMENU` and minimize/maximize styles, initial
client geometry and native caption/resize actions are preserved. While the alpha
lease is active, switching to a system title bar, hiding the title bar, removing
decorations or entering fullscreen is rejected without changing the window.
Opaque client renderers still use HWND flip and cannot be upgraded to alpha;
destroying an opaque renderer does not erase that HWND instance's flip history.

Alpha creation with an already attached application HMENU returns
`unsupported_platform`, retaining that menu and its commands. A nonempty
application `install_native_menu` is rejected before any menu/model/accelerator
mutation if any registered window holds a client-alpha lease. Empty clearing is
allowed; nested installation and new client-alpha acquisition are refused during
both install and clear callbacks. Releasing the last alpha owner lifts this menu
restriction. Direct external `SetMenu` is outside the supported route and is
detected by alpha status/repair, not repaired into a supported client menu.

The pre-existing opaque client-title-bar/HMENU layout is also not fixed here:
the custom client rectangle does not correctly reserve the native menu band.
This implementation neither hides/replaces the HMENU nor forces a framework
menu. System-title-bar DX12 alpha with native application menus remains supported.
See [DX12 backend scope](dx12-backend.md#bounded-ordinary-window-transparency) and
`.planning/2026-11-23-dx12-client-alpha/native-menu-findings.md` for the rejected
geometry/style candidates and the retained native-style contract.

The public `dx12_client_initial_alpha_test`, `dx12_client_alpha_lifetime_test`
and `dx12_client_alpha_recovery_test` cover initial geometry and hit tests,
title-row/body GPU and desktop alpha, fixed-mode rejection, resize, explicit
renderer scale 1/2, Mica/none, retained frames, destruction and two idle-device
removals. `dx12_client_menu_refusal_test` and
`dx12_client_menu_reentrancy_test` cover the refusal/transaction boundaries and a
system-title-bar positive control. These passed with WARP and DX12 debug
validation; they do not establish physical dragging, Snap Layouts interaction,
monitor-DPI migration or support for client-title-bar application menus.

`dx12_client_controls_test` additionally sends native maximize-button down/up
messages, observes real maximize/restore and minimize/restore state changes,
checks close-request cancellation, and creates an initially nonresizable client
window. It preserves alpha in GPU pixels while maximized and restores title-row
and body desktop blending afterward. This WARP/debug test is not physical mouse
or Snap Layouts acceptance.

## Linux (Wayland)

Source: src/platform/linux/wayland_window_title_bar.cpp,
wayland_window_title_bar_hit.cpp, wayland_protocol_xdg_toplevel_interactive.cpp
(hand-written `show_window_menu` opcode 4, `move` 5 and `resize` 6 from
xdg-shell.xml), and the hook in wayland_application_pointer.cpp.

- Decorations: with `zxdg_decoration_manager_v1`, client mode requests
  `client_side`. If the compositor answers `server_side` anyway, the applied
  mode falls back to `system` (the compositor draws its title bar) and the
  caption behaviors stop. Without the manager the surface is client-side
  already.
- Every pointer button press on a window records the seat and the press
  serial. In client mode, after the app has received the event:
  - left press in the resize band: `xdg_toplevel.resize(seat, serial, edge)`;
  - left press on `caption`: `xdg_toplevel.move(seat, serial)`, or toggle
    maximize when it is the second press within 400 ms and 4 px;
  - right press on `caption`: `xdg_toplevel.show_window_menu(seat, serial, x, y)`;
  - left release over the button region the press started on: minimize
    (`set_minimized`), toggle maximize (`set_maximized` / `unset_maximized`
    through `request_display_state`), or close (the close policy, source
    `window_manager`).
- Resize band: client-side decorations draw inside the surface, so the band
  is inside it too: 6 logical pixels on each edge, with corners 12 pixels
  along each edge. It is off while maximized or fullscreen and for
  non-resizable windows. `client_resize_border` reports it on all four sides.
- `perform_window_action` with `begin_move`, `begin_resize` or
  `show_window_menu` uses the last press serial; before any press it returns
  `supported = false`.
- Metrics: Wayland has no title bar metric. `caption_height` and
  `caption_button_size` are 0 and `reason` says so; pick your own row height.
- The cursor does not change over the resize band (the runtime sets the
  cursor from elements only).

## macOS

Source: src/platform/macos/macos_window_title_bar.mm (regions, input,
metrics, actions) and macos_window_chrome.mm (style mask).

- Style: client mode adds `NSWindowStyleMaskTitled |
  NSWindowStyleMaskFullSizeContentView`, hides the title and makes the title
  bar transparent. The content view covers the whole window; the native
  traffic lights stay over the top-left of the caption row and keep their
  own behavior (close through `windowShouldClose:` and the close policy,
  minimize, zoom, the full-screen menu). `WindowDescriptor::size` is the
  client size in both modes.
- `caption_buttons_leading_reserved_width` is the traffic lights' width
  plus the same margin on their right (78 at the 2026-11-21 measurement);
  keep caption content out of it. An app that sees a non-zero value should
  not draw its own minimize, maximize and close buttons:
  `examples/workbench` and `examples/client_title_bar` read the metrics
  after each frame and skip them.
- Input: AppKit sends every click to the content view, which no longer
  moves the window by itself (`mouseDownCanMoveWindow` is `NO`). After the
  app received a left press, the regions (last first) decide: `caption`
  starts `performWindowDragWithEvent:`, or on a double click follows the
  global `AppleActionOnDoubleClick` default (`Maximize` or unset: zoom,
  `Minimize`: miniaturize, `None`). A release over the button region the
  press started on minimizes, toggles zoom, or closes (source
  `window_manager`), for apps that draw their own buttons.
- Metrics: `caption_height` is the frame height minus
  `contentLayoutRect` (32 here), `caption_button_size` the traffic-light
  size (14 x 14). AppKit resizes from the window edge before the content
  view sees the event, so `resize_border` and `client_resize_border` are 0.
- Actions: `begin_move` drags with the mouse event being handled (`NSApp
  currentEvent`, a left press or drag in this window), otherwise it is
  not performed. `show_window_menu` (macOS has no window menu) and
  `begin_resize` are unsupported with a reason.
- Tests: `macos_client_title_bar_test` checks the style, metrics, actions,
  a close button release and a release outside the button, and switching
  back to the system title bar. Window dragging and double-click zoom were
  not driven by a test; `example_workbench` was captured on screen
  (2026-11-21, macOS 27.2.1, 1x): caption row flush with the window top,
  traffic lights at the left, title text after them, no system title bar.

X11 is not implemented. The X11 backend has no title bar support, so
`frame_metrics()` and the interactive actions return the base
`supported = false` with a reason. `_NET_WM_MOVERESIZE` would need the root
window and pointer-grab handling that the X11 window does not expose today.

## What was measured (2026-11-17)

Windows 11 Home China build 26300, real monitor DPI 144.
`win32_client_title_bar_test` also drives synthetic `WM_DPICHANGED` to 96
and 144:

| DPI | Resize band (px) | Resize band (logical) | `caption_height` (logical) | Button (logical) |
| --- | --- | --- | --- | --- |
| 96 | 8 (4 + 4) | 8 | 31 | 36 x 22 |
| 144 | 11 | 7.33 | 30 | 36 x 22 |

- Client rect: left, right and bottom inset by the band, top at the window
  top, at both DPIs; `WM_NCCALCSIZE(FALSE)` on a synthetic rect agrees.
- Maximized at the real DPI: window top -11, client top 0 (the work area
  top); the client rect lies inside the work area on every side.
- Hit tests at both DPIs: every region kind, every edge and corner, the last
  row of the top band (`HTTOP`) and the first row below it (`HTCAPTION`).
- `WM_NCMOUSEMOVE(HTMAXBUTTON)` reaches the app as `PointerMoved` at the
  button's logical position. Down and up become `PointerButton` events and
  maximize (`IsZoomed`); a second click restores; `HTCLOSE` produces
  `WindowCloseRequested`. Switching back to `system` restores the caption.
- `DWMWA_CAPTION_BUTTON_BOUNDS` still reports a button area in client mode
  (`250,0,470,44` in window pixels for a 480 x 320 window at 144 DPI).

Screenshots of `example_client_title_bar` (900 x 600 logical, 144 DPI,
dark mode; PNG captured from the screen, values per pixel):

- Before the DPI, scale and color fixes: window rect 900 x 600, client
  878 x 589; the caption row covered client columns 0..584 and rows 0..31,
  drawn as rgb(99,102,108) instead of rgb(32,34,38); the body was
  rgb(80,84,89).
- After: window rect 1372 x 911, client 1350 x 900; caption row
  rgb(32,34,38) over all 1350 columns and rows 0..47 (rows 0..1 show the
  Windows 11 window border, which overlaps the top client rows because there
  is no top frame); command box rgb(52,55,61) at x 495..854; body
  rgb(20,23,25).
- Moving the cursor to client 1317,37 turned exactly client columns
  1281..1349, rows 0..47 red (rgb(196,43,28)): the close button region
  (logical 854..900) is hit-tested and painted at the same place.
- No native DWM caption buttons appeared, with or without a Mica backdrop
  that extends the frame with -1 margins: the caption-button area (client
  x 989..1338, rows 2..47) was uniformly rgb(32,34,38) in both captures.
  The -1 margins are kept.

Wayland (`wayland_client_title_bar_test`, test compositor): client-side
decorations requested; a caption press sends `move` with that press's
serial; a second press sends `set_maximized`; a left-edge press sends
`resize` with edge 4 and its serial; a right press sends `show_window_menu`
at 50,16 with its serial; a press on the `client` control sends nothing;
the minimize button sends `set_minimized`; the close button produces
`WindowCloseRequested`.

`window_control_region_test`: region bounds follow the transform and clip;
element regions equal the painted box bounds and stay logical at scale 1.5;
`render_view` returns the `View::paint` regions, and with an element tree
the view regions then the element regions, which the runtime publishes as
painted; the base window reports unsupported actions with a reason.

## Not verified

- Only static screenshots and one hover were looked at. The Snap Layouts
  flyout over an app-drawn maximize button, drag feel, double-click
  maximize, Aero Snap and the right-click system menu were not tried by
  hand; only the hit-test codes and messages that drive them were checked.
- With no top non-client area, Windows 11 draws its 1-2 px window border
  over the top client rows (observed); whether that matters for app
  content there was not judged. Windows 10 is not tested.
- `begin_move`, `begin_resize` and `show_window_menu` on Windows are not
  exercised by tests (they enter modal loops).
- A maximized window with an auto-hide taskbar: no gap is reserved for
  revealing the taskbar.
- Wayland was tested only against the test compositor; no real compositor
  (WSLg Weston, Mutter, KWin) moved or resized a window.
