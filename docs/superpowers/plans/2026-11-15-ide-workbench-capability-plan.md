# IDE Workbench Capability Plan (Windows first)

Date: 2026-11-15
Consumer: CodeStudio Next (dual-pane editor + agent workbench, glass
material, light/dark following the system, mixed-DPI monitors).

These are framework capabilities. Product logic (document buffers, rope
storage, syntax highlighting rules, agent protocol) stays in the consumer.

## Build conventions for this plan

- `xmake.lua` is held at <= 4340 lines by older structure tests. New targets,
  file additions to existing targets, and tests go in focused modules under
  `build/xmake/capabilities/*.lua`, which the root includes with one glob.
  The architecture/header matrix test scans that directory.
- Every new `tests/architecture` or `tests/header_cleanliness` source must be
  listed in `scripts/ci/architecture-header-targets.txt`.
- Follow AGENTS.md: leaf headers, private `*_internal.hpp`, focused `.cpp`
  files, and structure tests with line limits for each new module.
- Gate per capability: full Windows debug `xmake build -a` and `xmake test`
  green, `git diff --check` clean, then one commit.
- Behavior that needs a real window, GPU, or human observation is recorded as
  unverified until it has been observed.

## Working rules (all capabilities)

- Scope per capability: Windows (Win32 + Vulkan) **and** Linux (Wayland +
  Vulkan; X11 only where an existing X11 backend already covers the area).
  macOS is planned only, in
  `2026-11-15-ide-workbench-macos-plan.md`; do not implement Cocoa/Metal code.
- Never pack several declarations or statements onto one line to satisfy a
  line-count tripwire. Split modules instead. If an existing tripwire must
  move, raise it explicitly in every test that guards that file and state the
  old and new limit in the commit message.
- Unsupported native services return typed results with `supported = false`
  and a reason; never fabricate success.
- Windows gate: from `D:\Dev\Projects\cgpui`, `xmake build -y -a` and
  `xmake test -y` fully green, `git diff --check` clean.
- Linux gate (WSL2 `archlinux`, running as root, `XMAKE_ROOT=y`): commit on
  Windows first, then in WSL
  `cd ~/cgpui-linux && git fetch -q origin && git checkout -q --detach origin/master`
  followed by `xmake build -y -a` and `xmake test -y`. `~/cgpui-linux` is a
  clone of the Windows repository; never edit it directly. Fix Linux failures
  on Windows and commit again. Install system packages with `pacman` only if
  an xmake package is not suitable, and record them in
  `docs/platform-dependencies.md`.
- Known environment flakes, not regressions: `x11_input_test` fails intermittently
  (most often under the parallel run; re-run it standalone up to three times); `win32_clipboard_unicode_test`
  fails when another process holds the clipboard. Re-run them standalone.
- Record anything only observable in a real window (visual material, title
  bar color, IME candidate placement) as "not visually verified" in the
  capability doc until observed.

## Capabilities

| ID | Capability | Windows | Linux (Wayland) |
|---|---|---|---|
| C0 | Commit DirectWrite glyph rasterization; capability build module | done `17fdc168` | n/a (fallback rasterizer unchanged) |
| C1 | Per-monitor DPI | done `b775b8c2`: PMv2 declared before the first window | done: reports `per_monitor_aware_v2` with `wp_fractional_scale_v1` + viewporter, else `per_monitor_aware` |
| C2 | System appearance: light/dark, accent, transparency, reduced motion, high contrast, change notifications; dark title bar | done `b775b8c2`, review `320091c9`; not visually verified | done: xdg-desktop-portal Settings over D-Bus, `SettingChanged` on the UI loop; typed unsupported without a portal (WSLg has none); no real portal observed |
| C3 | Window backdrop material, transparent Vulkan composition, opaque fallback | done (see `docs/window-backdrop.md`): `DWMWA_SYSTEMBACKDROP_TYPE` + frame extension, blur maps to acrylic, follows transparency setting; a creation-time backdrop prefers a GPU with premultiplied alpha (NVIDIA here, `applied = mica` measured); runtime-only requests keep the creation device; Mica observed on screen 2026-11-17 after the redirection-surface erase fix (dark material, painted pixels exact); Mica Alt / Acrylic not observed | done: `ext_background_effect_v1` / KDE blur bound when advertised, premultiplied surface, transparency setting follows the protocol; WSLg has neither (typed unsupported, opaque); tested against a fake global only |
| C4 | Custom title bar: hit-test regions (caption, buttons, resize borders) | done (see `docs/client-title-bar.md`): `WindowTitleBarMode::client`, regions from `PaintList` / `window_control_area`, `WM_NCCALCSIZE` keeps side/bottom frame (maximized inset), `WM_NCHITTEST` edges from `SM_CXSIZEFRAME + SM_CXPADDEDBORDER` then `HTCAPTION` / `HTMINBUTTON` / `HTMAXBUTTON` / `HTCLOSE`, NC mouse forwarded to the app, button actions on release; measured 8 px band at 96 DPI, 11 px at 144; Snap Layouts flyout not visually verified | done: client-side decorations requested, `xdg_toplevel.move/resize/show_window_menu` with the press serial, double-click maximize, 6 px in-surface resize band; tested against the test compositor only; X11 typed unsupported |
| C5 | Box shadow rendering in Vulkan | planned (shared renderer) | same |
| C6 | In-app backdrop blur for overlays | planned (shared renderer) | same |
| C7 | Native shaping and real metrics; caret / hit-test / selection use shaped advances | planned: DirectWrite `IDWriteTextAnalyzer` | planned: HarfBuzz + FreeType + fontconfig |
| C8 | IME composition cursor and clause attributes | planned: IMM32 `GCS_CURSORPOS` / `GCS_COMPATTR` | planned: `text-input-v3` preedit cursor |
| C9 | Workbench widgets: splitter, tabs, tree view, tooltip, context menu / popover | planned | planned (popups via `xdg_popup`) |
| C10 | SDK packages from a committed tree, manifest, consumer instructions | planned | planned |

## Order and rationale

C1 and C2 are small and unblock correct scaling and theming. C3 and C4 shape
the window itself. C5 and C6 complete in-app material. C7 is the largest item
and the main text-correctness blocker for an editor; it follows the window
work so visual checks can use the final chrome. C8 builds on C7 metrics. C9
uses all of the above. C10 packages the result.
