# IDE Workbench Capability Plan (Windows first)

Date: 2026-11-15
Consumer: CodeStudio Next (dual-pane editor + agent workbench, glass
material, light/dark following the system, mixed-DPI monitors).

These are framework capabilities. Product logic (document buffers, rope
storage, syntax highlighting rules, agent protocol) stays in the consumer.

## Build conventions for this plan

- The mcpp workspace owns libraries in `packages/*`, examples in `examples/*`,
  and test suites in `tests/*`. Add source files in their owning modules and
  register tests through the suite's `[test] discover` list.
- Every new `tests/architecture` or `tests/header_cleanliness` source must be
  listed in `scripts/ci/architecture-header-targets.txt`.
- Follow AGENTS.md: leaf headers, private `*_internal.hpp`, focused `.cpp`
  files, and structure tests with line limits for each new module.
- Gate per capability: full Windows debug `scripts/dev/build.ps1` and
  `scripts/dev/test.ps1` green, `git diff --check` clean, then one commit.
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
- Build system: mcpp (`mcpp.toml` workspace; see `docs/getting-started.md`).
  Builds and tests run in parallel. Register a new test through its suite's
  `[test] discover` (`tests/<suite>/mcpp.toml`), a new library source by
  the member that compiles it (`packages/<name>/mcpp.toml`).
- Windows gate: from `D:\Dev\Projects\cgpui`, `scripts/dev/build.ps1` and
  `scripts/dev/test.ps1` fully green (the wrappers configure `MCPP_HOME`;
  no `VSLANG` override is needed), `git diff --check` clean.
- Linux gate (WSL2 `archlinux`, running as root): commit on Windows first,
  then in WSL `GATE_REF=<branch> bash /mnt/d/WSL/gate-mcpp.sh` (it checks
  out `origin/<branch>` in `~/cgpui-linux` and runs `scripts/dev/build.sh`
  and `scripts/dev/test.sh`, `dev-lines` profile). `~/cgpui-linux` is a
  clone of the Windows repository; never edit it directly. Fix Linux
  failures on Windows and commit again. System libraries come from
  `pacman` and pkg-config; record new ones in
  `docs/platform-dependencies.md`.
- Former flakes, now fixed: `x11_input_test` (waited for no Expose before
  grabbing; `829821ea`) and `win32_clipboard_unicode_test` (clipboard
  listeners hold the clipboard after each change; the Win32 clipboard and the
  test now retry `OpenClipboard`); `clipboard_test` had the same
  unretried `OpenClipboard` in its own read / write / restore helpers
  (failed 1 in 3 locally on 2026-11-19) and now retries too.
  `win32_clipboard_files_test` still had unretried helpers (exit 4 / 9,
  5 in 425 looped runs); all three tests now share
  `tests/platform/win32_clipboard_test_open.hpp` (1 s retry). Measured
  listener holds: explorer.exe, cbdhsvc (clipboard history),
  DefenderSessionHelper, 20-75 ms each. The production
  `open_win32_clipboard` budget was 20 x `Sleep(5)`, i.e. 100-310 ms
  depending on timer resolution; it is now a 500 ms wall-clock deadline with
  2-32 ms backoff, guarded by `win32_clipboard_contention_test` (another
  process holds the clipboard 250 ms / 1.5 s). Note that `OpenClipboard(nullptr)`
  does not block another `OpenClipboard(nullptr)`; only windowed opens
  (real listeners) contend. Treat any failure as real.
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
| C5 | Box shadow rendering in Vulkan | done (see `docs/box-shadow-and-backdrop-blur.md`): `RenderFrame::draw_box_shadow(BoxShadowDraw)`, CSS semantics (sigma = blur / 2, spread grows radii, clipped out inside the box), analytic erf rounded-rect shader, drawn in paint order with clip, opacity, transform, DPI; pixel tests at scale 1 and 1.5 against the erf formula; inset not expressible in `BoxShadow` | shared renderer; headless geometry test on Linux, pixel test Windows only |
| C6 | In-app backdrop blur for overlays | done (see `docs/box-shadow-and-backdrop-blur.md`): `PaintList::draw_backdrop_blur` / `Style::backdrop_blur`, sigma in logical px; a frame with a backdrop renders to an offscreen scene split at each backdrop (box downsample + separable Gaussian on the region, rounded-mask composite with tint) and is copied exactly to the swapchain; no offscreen pass or allocation otherwise (asserted); each overlay blurs what precedes it; typed `backdrop_blur_support()` with tint-only fallback; observed in `example_backdrop_blur` at 1350 x 900 (inside 229..239 over full-contrast stripes); frame time within noise of the no-blur path (debug, CPU timers) | shared renderer; headless plan test on Linux; Wayland real window not checked |
| C7 | Native shaping and real metrics; caret / hit-test / selection use shaped advances | done `c4d4e250` (see `docs/native-text.md`): `AnalyzeScript` / `AnalyzeBidi`, `MapCharacters` fallback runs (CJK -> Microsoft YaHei UI), `GetGlyphs` + `GetGlyphPlacements` with OpenType features, font metrics, per-thread LRU shape cache, glyph-id rasterization (`CreateGlyphRunAnalysis`, quarter-pixel buckets); "Floating panel" width equals `IDWriteTextLayout` (175.355 px at 28 px); natural spacing and Chinese observed in `example_backdrop_blur`; color emoji still grayscale | done `174aa711`: fontconfig `FcFontSort` + charset coverage fallback, `hb_shape` per face/script run, FreeType metrics and `FT_Render_Glyph` grayscale by glyph id; width equals direct `hb_shape` (199.078 px); headless tests only, Wayland window not captured |
| C8 | IME composition cursor and clause attributes | done (see `docs/ime-composition.md`): `GCS_CURSORPOS`, `GCS_COMPCLAUSE`, `GCS_COMPATTR` converted to UTF-8 caret and `ImeClauseStyle` spans by IMM-free functions; widget draws thin / thick clause underlines and the composition caret from shaped advances; candidates (`CFS_EXCLUDE`) follow the composition caret; synthetic tests only, Microsoft Pinyin not verified | done: `zwp_text_input_v3` preedit `cursor_begin` / `cursor_end`, one `input` clause (protocol has none); cursor rectangle follows the composition caret; test compositor only |
| C9 | Workbench widgets: splitter, tabs, tree view, tooltip, context menu / popover | done (see `docs/workbench-widgets.md`): app-owned widgets with layout / append_elements / handle_event / paint, `WidgetTreeBuilder` publishes stable-id routing, focus and accessibility nodes (new roles tab_list, tab, tree, tree_item, menu, separator, tooltip; expanded / selected / level); splitter with 8 px hit area, capture, collapse, double-click restore, arrow keys; tab strip with measured titles, horizontal scroll overflow, dirty dot, middle-click close, drag reorder; virtualized tree (24 row nodes for 100 000 items), lazy children, range selection, type-to-select; tooltip 500 ms delay, flip, reduced-motion fade; in-window popover menu (GPUI deferred + anchored) with outside-click / Escape dismissal, focus restore, submenus, measured accelerator column, C5 shadow and optional C6 blur; observed in `example_workbench` (dark menu, light tooltip); UIA ExpandCollapse / SelectionItem patterns added later (see Framework fixes); screen readers not verified | shared widgets; built and tested headless; menus are in-window overlays (no `xdg_popup`); Wayland window not captured |
| C10 | SDK packages from a committed tree, manifest, consumer instructions | done (see `docs/sdk-packaging.md`): `scripts/ci/windows-sdk-package.ps1` builds `windows-package.ps1` with `/MDd` / `/MD` in a temporary `git worktree` of HEAD, `sdk-package.py` adds bundled lunasvg / plutovg, link order from the `example_workbench` link line, CRT check per library, the `src/ui` private headers `window_runtime.hpp` includes, `manifest.json` (commit, dirty = false, toolchain, Vulkan SDK, SHA-256 per file), CMake config (`cgpui::cgpui`) and README; zip via the Phase J archiver; CMake and xmake consumers built outside the repository and presented a frame (results in the doc) | done: `scripts/ci/linux-sdk-package.sh` in a clean clone, `pacman -Q` versions in the manifest, tar.gz; CMake and xmake consumers presented a frame on WSLg (dzn) |

## Framework fixes (after C9)

Defects found while building C9, fixed in the element, runtime, renderer
and platform layers. Each has a regression test that failed before the fix.

| Fix | Commit | Test |
|---|---|---|
| Layout bounds are window coordinates at any depth (GPUI model): containers place children with `Element::place_layout`, which moves the whole subtree; `ElementTree` lays out and hit-tests tree children; scrollable lists scroll item descendants; flex / vertical stacks paint children. The C9 `WidgetRootElement` workaround is removed | `d383f0f7` | `element_nested_coordinates_test` |
| Additional windows (all TestApp windows) own and publish their element tree: hit-test routing, cursor from the hit node, focus on press, accessibility updates, window-control regions | `5691a3bd` | `window_record_element_tree_test` |
| Vulkan: one render-finished semaphore per swapchain image, recreated with the swapchain (VUID-vkQueueSubmit-pSignalSemaphores-00067) | `8da87dc1` | `vulkan_semaphore_validation_test` (validation layer on, Windows and WSLg) |
| UIA ExpandCollapse and SelectionItem patterns for tree items, tabs and menu items; expand / collapse / select actions reach tree rows and tabs. AT-SPI has no Action / Selection interface yet, so Linux keeps states only | `b1465aee` | `win32_uia_item_pattern_test`, `workbench_accessibility_actions_test` |
| Popover menu surface opaque unless backdrop blur is requested | `a9bfffb5` | `workbench_popover_menu_test` |
| The element tree a view returns from `render()` is painted every frame, after `View::paint` (the view's canvas beneath its elements), in the root and additional windows; window control regions come from that paint. Static element trees carry no paint style and are not painted. `examples/hello_window` showed only the clear color before; its panel and accent bar are observed on screen after this and the next fix | `2fa034c1` | `view_element_tree_paint_test` |
| `v_stack()`, `row()`, `column()` keep their builder's box style: size fixes the content box, padding / margin surround it, background, border, radius, shadow and text style apply (hover / active state styles still do not) | `240124e2` | `stack_builder_style_test` |
| `scroll(state, child)` places the child (whole subtree) at the scroll offset, re-clamped to the measured sizes, and clips it to the viewport | `5310604a` | `scroll_element_offset_test` |
| Flex children resized by grow / shrink are laid out again at their final size, so their descendants follow the new box | `7fc15539` | `flex_grow_relayout_test` |
| Additional windows synthesize click / keyboard activation gestures, and actions fired while their events are handled resolve focus owner, view and handler context from that window | `4740d34e` | `window_record_action_dispatch_test` |
| `text(...)`, `text_input(...)` and `label(...)` paint their box style (shadow, background, border, radius) under the text and lay out a CSS content box (size, padding, margin) in which the text is drawn, hit-tested and caret-placed; the light bordered field in `examples/hello_window` is observed on screen. Image, icon, svg, button, item, checkbox and slider already painted their box | (this commit) | `leaf_element_box_style_test` |

## Order and rationale

C1 and C2 are small and unblock correct scaling and theming. C3 and C4 shape
the window itself. C5 and C6 complete in-app material. C7 is the largest item
and the main text-correctness blocker for an editor; it follows the window
work so visual checks can use the final chrome. C8 builds on C7 metrics. C9
uses all of the above. C10 packages the result.
