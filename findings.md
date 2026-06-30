# CGPUI GPUI-Core Findings

## 2026-07-01 Win32 System Clipboard Backend Merged

- Step 125 is merged on `master` at
  `389b9fb feat: add win32 system clipboard` and post-merge verified on
  Windows and WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 126-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch Linux full debug verification.
- Step 126 should start from `.worktrees/wayland-clipboard-skeleton` on
  `codex/wayland-clipboard-skeleton` and add a Wayland clipboard backend
  skeleton with graceful unsupported/no-seat behavior while preserving the
  shared `Clipboard` interface.

## 2026-07-01 Win32 System Clipboard Backend

- Step 125 keeps `MemoryClipboard` as the deterministic test/runtime fixture
  while making the Windows platform clipboard real through
  `CF_UNICODETEXT`.
- `create_platform_clipboard()` now remains platform-neutral at the API
  boundary: Windows returns a UTF-8 wrapper over the system clipboard, while
  non-Windows keeps the existing memory fallback until the Wayland skeleton in
  Step 126.
- The RED test needs a forced target rebuild after editing
  `tests/platform/clipboard_test.cpp`; otherwise xmake may reuse the previous
  binary and falsely report the old clipboard contract as passing.
- The WSL distro name on this machine is `archlinux`, not `Arch`; use
  `wsl -d archlinux ...` for Linux verification.

## 2026-07-01 Platform Cursor Application Merged

- Step 124 is merged on `master` at
  `74ad787 feat: apply platform cursors` and post-merge verified on Windows and
  WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 125-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 125 should add a Win32 system clipboard backend for UTF-8 text while
  keeping the existing memory clipboard path deterministic for runtime tests.

## 2026-07-01 Text Caret And Selection Paint Metadata Merged

- Step 123 is merged on `master` at
  `b0b9e00 feat: add text caret selection paint` and post-merge verified on
  Windows and WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 124-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 124 should connect runtime cursor state to Win32 and Wayland platform
  hooks while keeping clipboard, IME geometry, and cursor theme depth out of
  scope.

## 2026-07-01 Text Caret And Selection Paint Metadata

- Step 123 is implemented in `.worktrees/text-caret-selection-paint` on
  `codex/text-caret-selection-paint` and feature-worktree verified on Windows
  and WSL Arch Linux.
- Caret and selection are paint-list metadata only at this stage:
  `PaintCommandKind::text_selection`, `PaintCommandKind::text_caret`,
  `TextSelectionPaint`, `TextCaretPaint`, and matching `PaintList` fill helpers
  expose intent without adding Vulkan text drawing.
- `TextElement` uses the existing Step 122 deterministic fallback metrics:
  glyph width is `font_size * 0.5F`, selection geometry is derived from sorted
  byte offsets, and caret geometry is one pixel wide at the model cursor.
- A bound empty `TextModel` still emits caret metadata. Text content commands
  are emitted only when text is non-empty.
- `render_view(...)` skips text, text selection, and text caret commands until
  later text/glyph renderer work consumes them directly.
- Step 123 intentionally does not add shaping, glyph cache ownership, platform
  IME placement, platform cursor/clipboard behavior, or Vulkan text drawing.

## 2026-06-30 Font Descriptor And Font Size Style Merged

- Step 122 is merged on `master` at
  `58561b1 feat: add font size style` and post-merge verified on Windows and
  WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 123-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 123 should use the deterministic Step 122 `font_size` metrics and
  `TextPaint` font metadata to emit caret and selection paint metadata. Keep
  shaping, glyph cache ownership, platform IME placement, and Vulkan text
  drawing out of Step 123.

## 2026-06-30 Font Descriptor And Font Size Style

- Step 122 keeps font work as conservative public metadata: `FontDescriptor`
  currently carries a family string, `Style` and `StyleOverlay` carry font and
  `font_size`, and the element builder exposes `.font(...)` and
  `.font_size(...)` without adding platform font discovery yet.
- The default text metric contract is preserved at 16px height and 8px glyph
  width because `TextElement` now derives deterministic fallback metrics as
  glyph width `font_size * 0.5F` and height `font_size`.
- `TextPaint` now carries copied font metadata and font size alongside bounds,
  color, content, byte length, and clip metadata. This gives Step 123 a stable
  metric source for caret/selection paint metadata.
- Step 122 intentionally does not add shaping, glyph caches, font discovery,
  Vulkan text drawing, caret/selection commands, cursor, clipboard, or IME
  behavior. Those remain later planned slices.

## 2026-06-30 Text Paint Command Merged

- Step 121 is now merged at `cf180f4 feat: add text paint command` and
  post-merge verified on Windows and WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 122-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 122 should now add font descriptor and basic font-size style primitives
  on top of explicit text command metadata. Keep shaping, glyph caches, and
  Vulkan text drawing out of Step 122.

## 2026-06-30 Text Paint Command

- Step 121 separates UI text intent from rectangle painting by adding
  `PaintCommandKind::text`, `TextPaint`, and `PaintList::fill_text(...)`.
  Text payloads preserve bounds, color, copied UTF-8 content, byte length, and
  active clip metadata.
- `TextElement::paint(...)` now emits text commands instead of a placeholder
  solid rectangle. This keeps later font, caret, selection, glyph cache, and
  Vulkan text work attached to explicit text metadata instead of trying to
  infer text from rectangle fallbacks.
- `render_view(...)` intentionally skips text commands for now. That is better
  than converting text back to a solid rectangle because Step 152 will teach
  Vulkan to consume text/glyph metadata directly; until then text commands are
  observable in paint-list tests but not drawn by the simple rect renderer.
- Step 121 intentionally does not add font descriptors, font size style,
  caret/selection paint metadata, glyph caches, platform clipboard behavior, or
  real text shaping. Those remain Steps 122, 123, 125/126, and 149-152 work.
- Step 122 should build on this by adding font descriptor and font-size style
  metadata before caret/selection geometry tries to use text metrics.

## 2026-06-30 Back-40 Planning With Step 121 Active

- The后 40 步 remain Steps 129-168 and are still gated behind Steps 121-128
  plus the post-Step-128 Windows/WSL verification on `master`.
- The current planning anchor is the docs closeout commit
  `45a8dad docs: mark step 120 merged`; use
  `9aba0e6 feat: honor vulkan solid rect clips` as the Step 120 behavior
  commit, not the current `master` HEAD.
- Step 121 is already active in `.worktrees/text-paint-command` on
  `codex/text-paint-command`; future execution should finish and merge that
  branch before opening Step 122. Do not create a Step 129 worktree until the
  Step 128 exit contract passes.
- A useful后 40 步 plan needs more than branch names and target tests: each
  step now has an exit artifact and an explicit keep-out-of-scope note. This
  should reduce step creep in high-risk areas such as async, widgets, text
  shaping, Vulkan text drawing, multi-window lifecycle, accessibility, and the
  final parity audit.
- The distance estimate remains 8 implementation steps to Step 129 while Step
  121 is unmerged. After Step 121 merges and is post-merge verified, the
  distance becomes 7 implementation steps, Steps 122-128, plus the
  post-Step-128 verification gate.

## 2026-06-30 Back-40 Planning After Step 120 Merge

- The后 40 步 remain Steps 129-168, and they are now gated behind Steps
  121-128 plus the post-Step-128 Windows/WSL verification on `master`.
- Step 120 is no longer part of the active gate: it is merged at
  `9aba0e6 feat: honor vulkan solid rect clips` and post-merge verified on
  Windows and WSL Arch Linux.
- The next implementation slice is Step 121, text paint command metadata. It
  should replace text placeholder rectangles with command metadata without
  pulling in font descriptors, caret/selection rendering, or platform clipboard
  work early.
- The Step 168 outcome remains a practical Windows/Linux GPUI-core foundation
  plus a parity audit document. It is not full upstream GPUI parity, and
  macOS/Cocoa + Metal remains explicitly deferred.

## 2026-06-30 Vulkan Clip Rect Metadata

- Step 120 extends `SolidRect` with optional renderer-facing clip metadata so
  clipped paint commands can reach the backend without introducing a separate
  paint-command renderer API yet.
- `render_view(...)` must copy `PaintCommand::clip_rect` onto the `SolidRect`
  sent to `RenderFrame::draw_rect(...)`; otherwise the UI paint-list metadata
  added in Steps 53 and 119 stops before the renderer boundary.
- The current Vulkan solid-rect implementation uses `vkCmdClearAttachments`
  with a `VkClearRect`. Honoring clip metadata means computing the framebuffer-
  clamped intersection of `SolidRect::rect` and `SolidRect::clip_rect` before
  issuing the clear. This is equivalent to a scissor for the current clear-rect
  path and does not require a graphics pipeline yet.
- Win32 pixel sampling from an sRGB swapchain reports the clear color after
  conversion, so the outside-clip test should assert a dark clear-color range,
  not the raw linear `0.08/0.09/0.10` channel bytes.
- Step 121 can now focus on text paint commands. It should not revisit Vulkan
  clip handling or rounded-rect rasterization unless new text command tests
  force a shared command-vocabulary change.

## 2026-06-30 Back-40 Planning After Step 119 Merge

- The后 40 步 are still Steps 129-168, but the live entry gate has moved
  forward: Step 119 is merged and post-merge verified, so the remaining
  pre-back-40 work is Steps 120-128 plus post-Step-128 Windows/WSL
  verification.
- Step 120 should now be the next worktree. Its scope is Vulkan consumption of
  `PaintCommand::clip_rect` for solid rectangles; it should not expand into
  rounded-rect rasterization or text drawing.
- The Step 168 outcome remains a practical Windows/Linux GPUI-core foundation
  plus a parity audit document. It is not full upstream GPUI parity, and
  macOS/Cocoa + Metal remains explicitly deferred.

## 2026-06-30 Rounded-Rect Paint Command

- Step 119 keeps rounded rectangles as paint-list command metadata first. The
  Vulkan backend still receives the `solid_rect` fallback carried by each
  command; actual rounded drawing is intentionally left for later renderer
  work.
- Nonzero `Style::border_radius` on a styled background emits
  `PaintCommandKind::rounded_rect` with a `RoundedRect` payload preserving all
  four corner radii. Zero radius preserves the existing
  `PaintCommandKind::solid_rect` background behavior.
- `PaintList::fill_rounded_rect(...)` attaches the same active clip metadata as
  `fill_rect(...)`, so Step 120 can focus on Vulkan clip consumption without
  revisiting command storage.
- The next renderer slice should teach Vulkan to honor `PaintCommand::clip_rect`
  for solid rectangles. It should not implement real rounded-rect rasterization
  yet unless the plan is explicitly changed.

## 2026-06-30 Back-40 Planning After Step 118 Docs Closeout

- The后 40 步 are Steps 129-168 and remain gated behind completion of Steps
  119-128 plus post-Step-128 Windows and WSL verification.
- Current planning should anchor on the docs closeout commit
  `c0d38c7 docs: mark step 118 merged`, not only the Step 118 feature commit
  `9dfc2e7 feat: add layer elevation z order`.
- The intended Step 168 outcome is a practical Windows/Linux GPUI-core
  foundation plus `docs/gpui-core-api-parity.md`; it is not a claim of full
  upstream GPUI parity and it keeps macOS/Cocoa + Metal deferred.
- The immediate implementation action is still Step 119 in
  `.worktrees/rounded-rect-paint-command`, starting with RED tests for
  border-radius metadata in rounded-rect paint commands.

## 2026-06-30 Layer/Elevation Z Order Merged

- After Step 118 merged, the remaining pre-back-40 gate is Steps 119-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification.
- Step 119 should build on the current paint command stream by preserving
  border-radius metadata in a rounded-rect command shape first. Avoid changing
  Vulkan rounded drawing behavior until the command metadata is pinned by RED
  tests.

## 2026-06-30 Layer/Elevation Z Order

- Step 118 maps layer/elevation onto deterministic paint order by adding
  `Style::layer`, `StyleOverlay::layer`, builder `.layer(...)`, and
  `Element::layer()`/`z_order()`.
- Explicit nonzero `z_index` remains the compatibility override. The effective
  paint key is `z_index` when it is nonzero; otherwise it is `layer`.
- Layer and z-index metadata need to live on the final `Element`, not only on
  `Style`, because parent paint ordering sees the outer wrapper after click,
  pointer, key, and focusable wrappers are applied.
- Stable sort remains important: siblings with the same effective `z_order()`
  continue painting in authored insertion order, preserving the existing
  deterministic z-index contract.

## 2026-06-30 Back-40 Planning After Step 117 Docs Closeout

- The后 40 步 remain Steps 129-168 and should not preempt the active Step
  118-128 queue.
- The current planning anchor is `c3b6ecb docs: mark step 117 merged`; use
  `210c85d feat: add absolute positioning insets` only as the Step 117 feature
  commit reference.
- The remaining route to Step 129 is 11 implementation steps, Steps 118-128,
  plus post-Step-128 targeted verification, Windows full debug, WSL Arch full
  debug, and a clean tracked/staged worktree.
- The back-40 plan now explicitly records Steps 115 and 116 as gate-complete
  alongside Step 117. This matters because Steps 139-148 widget work and
  Steps 149-158 rendering work depend on the full flex/alignment/grow/shrink/
  absolute/layer layout substrate being complete before Step 129 starts.

## 2026-06-30 Absolute Positioning And Insets Merged

- After Step 117 merged, the remaining pre-back-40 gate is Steps 118-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification.
- Step 118 should build on Step 117's layout substrate by adding
  layer/elevation ordering as deterministic metadata first. Avoid changing
  absolute child sizing or adding right/bottom anchoring unless Step 118 tests
  explicitly require it.

## 2026-06-30 Absolute Positioning And Insets

- Position and inset need to live on the final `Element`, not only on `Style`,
  because parent stack/flex layout reads each built child after behavior
  wrappers like click, pointer, key, and focusability have been applied.
- Default `Position::relative` preserves all previous stack/flex layout.
  Absolute children are measured for their own size but excluded from normal
  flow size, gap spacing, flex grow/shrink allocation, justification, and
  alignment.
- Step 117 intentionally gives `left` and `top` placement semantics first.
  `right` and `bottom` are stored in `EdgeSizes` for API continuity and later
  anchoring work, but they should not drive layout until a later step adds
  explicit right/bottom behavior and tests.
- Stack and flex layout should place absolute children after the parent output
  size is known. This keeps normal-flow children deterministic and gives Step
  118 layer/elevation work a stable layout substrate.

## 2026-06-30 Back-40 Planning After Step 116 Docs Closeout

- The后 40 步 are still Steps 129-168, and the current planning anchor should
  be the docs closeout commit `1e542bd docs: mark step 116 merged`, not only
  the Step 116 feature commit `2806a4a feat: add flex grow shrink layout`.
- Step 117 already has an active worktree at
  `.worktrees/absolute-position-insets` on `codex/absolute-position-insets`.
  Future execution should continue that branch through verification and merge
  instead of creating a new Step 117 branch or starting Step 129.
- The useful post-Step-128 delivery target is four staged outcomes:
  context/entity/async ergonomics, keyed reusable widgets and style cascade,
  backend-ready text/render metadata, and Win32/Wayland platform closure plus a
  parity audit. Step 168 should remain the audit/handoff milestone, not a claim
  of full upstream GPUI parity.
- From current `master`, the back-40 entry is still gated by 12 implementation
  steps, Steps 117-128, plus post-Step-128 targeted, Windows full debug, and
  WSL Arch full debug verification.

## 2026-06-30 Flex Grow And Shrink Merged

- After Step 116 merged, the remaining pre-back-40 gate is Steps 117-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification.
- Step 117 should start from a fresh `.worktrees/absolute-position-insets`
  worktree and keep absolute positioning/insets as layout metadata first,
  before Step 118 layers/elevation and Step 119+ renderer command hardening.

## 2026-06-30 Flex Grow And Shrink

- Flex grow and shrink factors need to live on `Element`, not only on
  `Style`, because the parent `FlexElement` reads each final child element
  during layout.
- Builder-created behavior wrappers must copy the flex factors onto the
  outermost wrapper (`ClickElement`, `PointerElement`, `KeyElement`, and
  `FocusableElement`). Otherwise the parent flex container would see zero
  factors when a child has handlers or focusability.
- Default grow/shrink factors remain `0.0F`, preserving previous flex layout
  for existing children unless authors explicitly opt in.
- Shrink tests that constrain only height must still provide a nonzero max
  width when using aggregate `Size` initialization, because omitted width
  fields default to zero and can collapse output width before shrink behavior
  is evaluated.

## 2026-06-30 Back-40 Planning Hardening

- The后 40 步 should be treated as an execution-ready continuation of the
  current Step 128 gate, not as a separate roadmap. The useful completion
  target is a practical Windows/Linux GPUI-core foundation with public
  context/entity/global APIs, reusable keyed widgets, deterministic
  text/render metadata, platform hooks, and an explicit parity audit.
- Step 168 remains an audit milestone rather than a full GPUI parity claim.
  The audit should be the authoritative place for implemented, partial,
  missing, and Mac/Metal-deferred areas.
- The back-40 non-goals are important safety rails: do not begin macOS/Metal
  parity, do not replace Vulkan/xmake, do not require a full shaping stack
  before deterministic text contracts exist, do not create a separate widget
  framework, and do not combine small steps to reduce commit count.
- Step 129 should start from a verified `master` with a fresh
  `.worktrees/context-authoring-alias` worktree. Its first RED should be a
  public `Context<T>` authoring alias compile failure plus a runtime test that
  proves the alias preserves existing `ViewContext` helper behavior.

## 2026-06-30 Flex Alignment And Justification

- Flex alignment and justification can stay entirely in shared style/element
  code for Step 115. No runtime or platform-specific behavior is needed.
- `JustifyContent::start` preserves the old flex layout behavior. `center` and
  `end` use only positive constrained free space as initial main-axis offset,
  and `space_between` keeps the authored `gap` as the base gap while
  distributing extra free space between children.
- `AlignItems` uses the final constrained cross-axis size, so rows align child
  y origins and columns align child x origins. Cross-axis free space is clamped
  at zero, preserving previous overflow behavior for children larger than the
  constrained container.
- Measuring children before assigning final bounds is necessary because the
  final constrained flex size controls both main-axis justification and
  cross-axis alignment.

## 2026-06-30 Back-40 Planning After Step 114 Closeout

- The后 40 步 remain Steps 129-168 and should still be treated as a
  post-Step-128 execution queue, not the active branch queue while Steps
  115-128 are incomplete.
- Current `master` is anchored at `ac745f8 docs: mark step 114 merged`; the
  Step 114 behavior commit is `d78a017 feat: clip hidden overflow hit testing`.
- Step 115 already has a worktree at `.worktrees/flex-alignment-justification`
  on `codex/flex-alignment-justification`, and its baseline targeted tests have
  passed. The next practical action is to add Step 115 RED tests there, not to
  recreate the worktree or start Step 129.
- The route to Step 129 is 14 incomplete implementation steps, Steps 115-128,
  plus post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. The route through Step 168 is 54 implementation steps plus
  checkpoint reviews after Steps 138, 148, 158, and 168.
- The back-40 order remains context/entity/global/async first, keyed
  reconciliation/widgets/style cascade second, text/font/renderer diagnostics
  third, and Windows/Wayland platform closure plus parity audit last.

## 2026-06-30 Hidden Overflow Hit Testing

- `StyledElement` needed its own `hit_test(...)` override because the inherited
  `Element::hit_test(...)` only checked the styled wrapper's layout bounds; it
  neither forwarded to the owned child nor applied `Overflow::hidden` clip
  semantics.
- Hidden overflow hit testing should mirror the existing paint clip contract:
  if `Style::clip_rect` is present, use it as the hit-test clip; otherwise use
  the styled element's layout bounds. Points outside that clip return no hit
  before checking child or self hits.
- Visible overflow must keep checking the child before falling back to the
  styled element itself. This preserves current authoring behavior where a
  child can be hit outside the wrapper's own bounds when overflow remains
  visible.
- Runtime coverage for an internally owned child is limited by
  `ElementTree::get(...)`: a child owned inside `StyledElement` is not a tree
  node, so runtime route assertions should verify root/no-target clipping
  behavior unless a later step promotes internal children into the tree.
- Keep `ScrollElement::hit_test(...)` wrapper-targeting unchanged. Step 113
  depends on scroll routing finding the scroll viewport wrapper so the runtime
  can locate the bound `ScrollState`.

## 2026-06-30 Back-40 Planning At Step 113 Docs Closeout

- The后 40 步 are still Steps 129-168, and the current anchor should be the
  docs closeout commit `2251d44 docs: mark step 113 merged`, not only the Step
  113 behavior commit `7a2ef39 feat: route scroll events to scroll state`.
- The gate into Step 129 is now exactly Steps 114-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- The execution order should stay sequential and banded: Steps 129-138 for
  context/entity/global/async, Steps 139-148 for keyed reconciliation/widgets,
  Steps 149-158 for text/font/renderer diagnostics, and Steps 159-168 for
  Windows/Wayland platform closure plus the parity audit.
- The next practical implementation action remains Step 114 in a fresh
  `.worktrees/hidden-overflow-hit-testing` worktree, not Step 129.

## 2026-06-30 Scroll Routing

- `PointerScrolled` routing belongs after route target selection and ancestry
  refresh but before generic element dispatch/view fallback. That lets scroll
  state consume wheel/trackpad deltas without invoking the view when a bound
  `ScrollElement` owns the viewport under the pointer.
- `ScrollElement` should hit-test as the scroll viewport wrapper for now. Its
  child is owned internally rather than indexed in `ElementTree`, so returning
  the child id can leave `WindowRuntime::routed_element(...)` unable to find
  the element that actually owns the `ScrollState`.
- The scroll delta should be applied through `ScrollState::scroll_by(...)`
  instead of setting offsets directly so existing viewport/content-size clamp
  semantics remain the single source of truth.
- A no-scroll route remains a soft-fail path: if no routed `ScrollElement` is
  found, the runtime continues to normal element dispatch and view fallback.


## 2026-06-30 Back-40 Planning After Step 112 Closeout

- The后 40 步 are Steps 129-168 and remain a post-Step-128 follow-on queue, not
  the active branch queue while Step 113-128 are still incomplete.
- Current `master` is anchored at `ed948a7 docs: mark step 112 merged`; Step
  113 already has a worktree at `.worktrees/scroll-routing` on
  `codex/scroll-routing`, so the correct next action is to continue that
  worktree's RED/GREEN cycle.
- Step 129 is 16 implementation steps away: Steps 113-128 plus post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification. Step 168
  is 56 implementation steps away from current `master`, plus the four band
  checkpoint reviews.
- The back-40 band order remains context/entity/global/async first, keyed
  reconciliation/widgets/style cascade second, text/font/renderer diagnostics
  third, and Windows/Wayland platform closure plus the parity audit last.


## 2026-06-30 Focus Traversal

- Step 111 can reuse `ElementTree::enabled_preorder_ids()` as the traversal
  source. That keeps disabled descendants out of traversal before checking
  `Element::focusable()`, so disabled focusable-looking elements do not receive
  Tab focus.
- Focus traversal belongs in the keyboard-key pre-dispatch path: after a
  successful traversal, the current event route should be retargeted to
  `keyboard_focus_element_owner_` and ancestry refreshed so key bindings,
  routed element handlers, view fallback, and after-event callbacks all see the
  updated focused element.
- Shift+Tab can share the same traversal helper as Tab by passing a reverse
  flag. With no current focus, forward traversal chooses the first enabled
  focusable element and reverse traversal chooses the last one; with current
  focus, both directions wrap.
- The helper should ignore Tab variants with control, alt, or super modifiers
  so future app/window shortcuts can still bind those chords. Shift remains the
  reverse traversal selector.

## 2026-06-30 Back-40 Planning With Step 111 Active

- The后 40 步 still mean Steps 129-168, and they remain a post-Step-128
  follow-on queue. They should not preempt the active Step 111-128 gate.
- Current `master` is anchored at `d52ce80 docs: mark step 110 merged`; the
  Step 110 behavior commit is `10f2dd3 feat: add event propagation phases`.
- Step 111 has landed on `master`; planning should point future workers at
  Step 112, not at recreating `.worktrees/focus-traversal` or starting Step 129
  early.
- From current `master`, Step 129 is 17 implementation steps away: Steps
  112-128 plus post-Step-128 targeted, Windows full debug, and WSL Arch full
  debug verification.
- Step 168 is still 57 implementation steps away from current `master`: Steps
  112-168 plus the four band checkpoint reviews after Steps 138, 148, 158, and
  168.
- The back-40 band order remains context/entity/global/async first, keyed
  reconciliation/widgets/style cascade second, text/font/renderer diagnostics
  third, and Windows/Wayland platform closure plus parity audit last.

## 2026-06-30 Back-40 Planning After Step 110 Merge

- The后 40 步 remain Steps 129-168. They should still be treated as the
  post-Step-128 follow-on queue, not as the active implementation queue.
- Current `master` is anchored at `10f2dd3 feat: add event propagation phases`;
  Steps 89-110 are merged and post-merge verified on Windows and WSL Arch
  Linux.
- From this state, Step 129 is 18 implementation steps away: Steps 111-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. Step 168 is 58 implementation steps away: Steps 111-168 plus
  the four band checkpoint reviews.
- The next active implementation should remain Step 111, focus traversal over
  enabled focusable elements with Tab and Shift+Tab actions. Starting Step 129
  before focus, scroll, layout depth, renderer/text metadata, cursor/clipboard,
  IME geometry, and the public-prelude demo rewrite land would destabilize the
  follow-on context/widget/platform plan.
- The back-40 band order should stay: context/entity/global/async, keyed
  reconciliation/widgets/style cascade, text/font/renderer diagnostics, then
  Windows/Wayland platform closure plus the parity audit.

## 2026-06-30 Event Propagation Phases

- Step 110 uses `EventRoute::element_ancestry` as the dispatch route. The order
  remains target-to-root: the target element handles first, then each ancestor
  can handle the same event until one returns consumed or cancelled.
- `ElementEventContext::target_element_id` intentionally remains the original
  route target for every bubbling phase. This lets ancestor handlers know which
  descendant was hit/focused without changing the public context shape yet.
- Disabled elements remain part of route ancestry for diagnostics, but their
  handlers are skipped during bubbling. Dispatch continues to later enabled
  ancestors and only falls back to the root view when every routed element
  returns unhandled or is skipped.
- A route without `element_ancestry` still falls back to the route target id,
  preserving compatibility for legacy routes while Step 109's ancestry refresh
  is now the normal path.

## 2026-06-30 Back-40 Planning After Step 109 Merge

- The post-Step-128 "back 40" remains Steps 129-168. It is an execution-ready
  follow-on queue, not the active branch queue while Step 110-128 are still
  incomplete.
- Current `master` is anchored at `08d0fef docs: mark step 109 merged`; the
  Step 109 feature commit is `74df1df feat: add event route ancestry`.
- From this state, Step 129 is 19 implementation steps away: Steps 110-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. Step 168 is 59 implementation steps away: Steps 110-168 plus
  the four band checkpoint reviews.
- The back-40 order should stay banded: context/entity/global/async first,
  keyed reconciliation/widgets/style cascade second, text/font/renderer
  diagnostics third, and Windows/Wayland platform closure plus parity audit
  last. This order avoids asking widgets or platform hooks to invent missing
  shared runtime contracts.

## 2026-06-30 Event Route Ancestry Metadata

- Step 109 keeps ancestry as a route snapshot, not a propagation behavior
  change. `EventRoute::element_ancestry` is ordered target-to-root so Step 110
  can handle the target first and then bubble through ancestors without
  recomputing parent links.
- `EventRoute::view_ancestry` is also ordered target-to-root. A normal root
  event records only `ViewId{1}`; a hit on a registered `ChildViewElement`
  records the child `ViewId` followed by the root view. The current event still
  dispatches through the existing root view fallback until Step 110 adds real
  propagation phases.
- Runtime route refresh must happen after the final target is selected,
  because keyboard focus, pointer capture, and hit testing can each choose a
  different element or view target. View-target pointer capture clears the
  element target before ancestry is recalculated so stale element metadata does
  not leak into view-routed events.
- Child-view route targeting only upgrades to the child view when the placeholder
  references a registered live view. Removed or missing child views still leave
  the event on the root route, matching the view registry soft-fail contract.

## 2026-06-30 Child-View Placeholder

- Step 108 is intentionally metadata-only. `ChildViewElement` stores a
  `ViewId`, lays out to a constrained placeholder size, participates in
  hit-testing as a normal element, and paints nothing for now.
- Real nested view rendering, parent/child view ancestry, and event propagation
  across view boundaries remain Step 109/110 work. Keeping Step 108 small gives
  those steps a concrete element marker without forcing their routing semantics
  early.
- `ViewId` belongs in `element.hpp` now because child-view placeholders are
  authorable through element builders and public prelude helpers. Keeping a
  duplicate `ViewId` in `ui.hpp` would either make `element.hpp` depend on the
  runtime header or split the public id type.
- The runtime integration test should only prove that a registered child
  `ViewId` can be referenced by an installed placeholder element while
  `WindowRuntime::find_view(...)` still resolves the view. It should not call
  the child view's `render(...)` yet.
- After the Step 108 merge, Step 109 is the right next slice because the
  placeholder now gives event routing a concrete element marker that can carry
  both `ElementId` ancestry and child `ViewId` metadata without needing nested
  rendering first.

## 2026-06-30 Back-40 Planning While Step 108 Active

- The後 40 步 are still Steps 129-168, but the active implementation path has
  not crossed the Step 128 gate yet. Step 108 already has a worktree, so the
  next engineering action should continue `.worktrees/child-view-placeholder`
  instead of creating a Step 129 branch.
- The post-Step-128 plan should be treated as an execution-ready queue with
  branch slugs, RED test intents, targeted commands, Windows full debug, and
  WSL Arch full debug for every step. It is not just a roadmap, but it is still
  blocked by Steps 108-128.
- The current pre-back-40 gate count is 21 implementation steps: Step 108
  through Step 128. The high-risk gate items are event ancestry/propagation,
  text paint metadata, Vulkan clip/text command handling, cursor/clipboard
  backends, IME geometry, and the public-prelude demo rewrite.
- Step 168 should close with a parity audit, not a claim of full upstream GPUI
  equivalence. The useful audit split is implemented, partial, missing, and
  Mac/Metal-deferred areas for the Windows/Linux track.

## 2026-06-30 View Registry Skeleton

- Step 107 is intentionally a registry/lifetime skeleton, not child-view
  rendering. It gives later child-view placeholders, ancestry routing, and
  multi-window work a single view lookup path before those behaviors exist.
- The root view is registered as borrowed and non-removable at `ViewId{1}`.
  Additional borrowed views can be registered by reference, and owned views can
  be registered through `std::unique_ptr<View>`.
- App-opened root views should use the general view registry instead of a
  separate root-view ownership map. This keeps `AppContext::open_window(...)`,
  `app_opened_window_root_view(...)`, `WeakView` upgrade, and future child-view
  lookup on the same storage contract.
- Removing a registered non-root view should make `find_view(...)`,
  `is_view_id_allocated(...)`, and `WeakView` upgrade soft-fail for that id.
  Plain ids from `allocate_view_id()` remain compatible with the older
  monotonic allocation contract until a later view lifecycle step replaces it
  with full generation/removal semantics.

## 2026-06-30 Back-40 Planning After Step 107

- The "后40步" plan is now best understood as a post-Step-128 execution plan,
  not an immediate branch queue. With Step 107 feature-worktree verified, the
  remaining gate into Step 129 is Steps 108-128 plus final Windows and WSL Arch
  Linux verification on `master`.
- Step 129 should start only after the child-view, ancestry, propagation,
  focus, scroll, layout-depth, render-command, cursor, clipboard, IME, and
  public-demo slices have landed. Starting it earlier would make the context
  and widget APIs depend on unstable or placeholder runtime surfaces.
- The four 10-step bands should remain ordered: context/entity/async first,
  keyed widgets and style cascade second, text/font/renderer diagnostics third,
  and Windows/Wayland platform closure plus parity audit last. This order gives
  each band a usable substrate instead of asking later platform work to invent
  missing shared contracts.
- Because Steps 129-168 all touch public headers, shared runtime behavior,
  renderer contracts, or platform-neutral event data, every step should keep
  WSL Arch full debug in its normal definition of done even when the visible
  feature name is Win32-specific.

## 2026-06-30 App-Opened Root View Lifecycle

- Step 106 should remain a storage/lifecycle slice, not real multi-window
  platform creation. `AppContext::open_window(WindowOptions,
  std::unique_ptr<View>)` records the window descriptor, allocates a distinct
  `ViewId`, and transfers root view ownership into `WindowRuntime`; `run_app`
  still creates only the one platform/native window from the primary
  descriptor.
- Keeping `open_window(WindowOptions)` source-compatible with an empty
  `root_view_id` preserves the Step 105 skeleton path while allowing callers
  that supply root view ownership to receive an allocated id and query the
  stored view during setup and frame callbacks.
- The runtime-owned root view container naturally destroys app-opened root
  views when `run_app` returns and the `WindowRuntime` is destroyed. No
  explicit cleanup at `WindowRuntime::run()` start or end is needed for this
  slice, and clearing there would risk erasing setup-time app-opened windows.
- The initial Step 106 GREEN failure was test-side: the after-frame callback
  captured a setup-local pointer variable by reference. Capturing the pointer
  value keeps the test checking runtime lifecycle behavior instead of a
  dangling test reference.

## 2026-06-30 Back-40 Planning After Step 105 Merge

- The "后40步" plan is now an execution-ready follow-on queue for Steps
  129-168, but the active implementation path is still Steps 106-128. Starting
  Step 129 early would skip root-view lifecycle storage, the view registry,
  child-view placeholders, route ancestry, propagation, focus/scroll/layout
  depth, render command hardening, cursor/clipboard/IME surfaces, and the
  public-prelude demo rewrite.
- The clean handoff into Step 129 requires 23 remaining implementation steps:
  Steps 106-128, then post-Step-128 targeted verification, Windows full debug,
  WSL Arch full debug, and a clean `master` except the known untracked
  `.vscode/`.
- The back-40 should be reviewed in four checkpoint bands: Step 138 for
  context/entity/global/async APIs, Step 148 for keyed widgets and style
  cascade, Step 158 for text/render diagnostics, and Step 168 for
  Windows/Wayland platform closure plus the GPUI-core parity audit.

## 2026-06-30 Render Invalidation Observability

- Render invalidation should be a strict superset of layout and paint
  invalidation: `request_render()` sets render/layout/paint and schedules a
  redraw, while existing `request_layout()` and `request_paint()` keep the new
  render bit false.
- The first after-render observation point belongs immediately after
  `View::render(...)` returns and any non-null root element is installed, before
  layout, paint, invalidation clearing, and frame index increment. This lets
  callers inspect the render sequence and installed root id while the frame is
  still in progress.
- `RenderRecord::root_element_id` should report the root installed by the
  current render pass. If a view returns an empty `AnyElement`, the record can
  still report sequence, view id, and viewport size with no installed root id.
- Resetting render sequence and last render record at `WindowRuntime::run()`
  keeps each run deterministic and matches the existing event/frame sequence
  reset behavior.

## 2026-06-30 Steps 129-168 Execution Matrix

- The follow-on 40 steps should start only after Step 128 has a clean
  post-merge Windows and WSL baseline; otherwise context/widget/platform work
  will sit on top of unstable render, model, routing, text, or demo surfaces.
- The useful next level of planning detail is operational rather than more
  conceptual: each step now has a branch slug, first RED test intent, and
  targeted command so the existing RED/GREEN/merge cadence can continue
  without re-deciding the shape of the step every turn.
- All Steps 129-168 should keep WSL Arch full debug in the per-step definition
  of done, including Win32-looking steps, because these slices almost always
  introduce shared public headers, runtime contracts, or platform-neutral event
  data consumed by the Linux/Wayland path.
- The dependency order should stay banded: context/entity/global/async before
  widgets, keyed/lifecycle/style/focus/widgets before renderer text depth,
  renderer/text/diagnostics before platform completion and the final parity
  audit.

## 2026-06-30 Runtime Render Pass

- The narrow Step 97 integration point is `WindowRuntime::handle_redraw()`:
  rendering before the existing owned-tree layout pass lets the runtime reuse
  current `ElementTree::layout_root`, hit testing, routed-element lookup, and
  invalidation cleanup behavior.
- `View::render(ViewContext&)` takes a non-const lvalue context, so the runtime
  must materialize `ViewContext render_context = context();` before calling the
  hook instead of passing the temporary result of `context()` directly.
- A non-null render result should replace the runtime owned tree through the
  same `set_element_tree(...)` path used by `ViewContext`; this keeps
  `element_root()` and pointer routing consistent with manually installed
  trees.
- Existing `paint(...)` is still invoked by `render_view(...)` after the render
  tree is installed, preserving the compatibility contract until later steps
  route paint command generation through the rendered element tree.

## 2026-06-30 View Render Hook Skeleton

- `ViewContext` must be available before `View` is declared because the new
  virtual render hook uses the GPUI-like context spelling directly in the base
  class.
- The first render hook should be additive: the default `View::render` returns
  an empty `AnyElement`, while the existing `paint(...)` method remains pure
  virtual and source-compatible for old runtime and demo paths.
- Step 96 deliberately does not install the rendered tree into the runtime.
  That behavior remains Step 97 so the API hook can be verified separately
  from redraw, layout, hit testing, and owned element tree replacement.
- Calling `request_paint()` or `request_layout()` from render already works
  through `ViewContext` because the alias still forwards to
  `WindowRuntimeContext`; Step 98 can add render-specific invalidation and
  observability without changing the Step 96 hook shape.

## 2026-06-30 Steps 129-168 Planning Refresh

- The forward queue should be treated as a post-Step-128 plan, not as the next
  active implementation queue: Step 96 still gates the current track because
  `View::render`, runtime render-tree installation, model/app lifecycle,
  bubbling/focus/scroll/layout depth, render commands, cursor/clipboard/IME
  surfaces, and the public-prelude demo all feed the APIs used by Steps
  129-168.
- The most useful planning boundary is an execution gate rather than a second
  competing active plan. This keeps the follow-on route visible while preserving
  the current Step 96-128 RED/GREEN workflow.
- After Step 128, the first 10 follow-on steps should bias toward context,
  entity, global, scoped action, subscription, deferred, timer, async, batching,
  and diagnostics APIs before widget or renderer expansion.

## 2026-06-30 Style-State Overlays

- A separate `StyleOverlay` is necessary because `Style` has non-optional
  scalar/POD fields where zero is a valid authored value; optional overlay
  fields preserve "unset" versus "override to zero" semantics.
- Step 95 should stay authoring/data-only: `resolved_style(...)` is
  deterministic and testable, but runtime hover/focus/disabled application can
  wait for later render/style resolution work.
- The merge order is base, hover, focus, disabled. This lets disabled state win
  over active interaction state while still inheriting focus/hover fields that
  disabled does not override.
- `StyledElement` can store a full `StyleState` while keeping its old
  `style()` API as a base-style accessor. That preserves existing layout,
  paint, z-index, and tests while making state overlays available to later
  runtime/style-cascade slices.

## 2026-06-30 Steps 129-168 Forward Plan

- Steps 129-168 should remain a follow-on queue until Steps 95-128 finish,
  because the later context/widget/platform work depends on `View::render`,
  model helpers, propagation, text paint commands, cursor/clipboard/IME
  surfaces, and the public-prelude demo rewrite.
- The highest-leverage next 40 steps after Step 128 are not more authoring
  sugar; they are API-shape completion and runtime maturity: typed contexts,
  entity handles, globals, scoped actions, subscription ownership, deferred
  work, timers, async completions, keyed reconciliation, style cascade,
  widgets, text/font renderer depth, multi-window storage, and Win32/Wayland
  lifecycle hooks.
- The riskiest region is Steps 149-168, where text/font rendering and platform
  hooks cross from deterministic metadata into Vulkan, Win32, and Wayland
  behavior. Keep each step command-driven, fakeable in tests, and backed by
  Windows plus WSL verification.

## 2026-06-30 Pointer Handler Shortcuts

- Pointer handler shortcuts fit the existing wrapper pattern: keep element
  layout, paint, and hit testing transparent while intercepting only matching
  `PointerButton` or `PointerMoved` events.
- `.on_click(...)` remains source-compatible and still means pointer press; the
  new `.on_pointer_down(...)`, `.on_pointer_up(...)`, and
  `.on_pointer_move(...)` handlers expose concrete event data for authoring
  paths that need button or position details.
- Pointer handlers should return their handler result only when consumed or
  cancelled; otherwise the wrapper can continue forwarding to its child, which
  keeps future bubbling/ancestor propagation work from being boxed in too early.

## 2026-06-30 Element Builder Fluent Style Shortcuts

- Builder-level style shortcuts should remain thin mutations of the builder's
  retained `Style`; this keeps `.style(Style)` source-compatible and avoids a
  second style storage path.
- `.size(Size)` and `.size(float, float)` need to update both
  `style_.preferred_size` and `size_` when the builder kind is `fixed_size`,
  because `FixedSizeElement` build output reads `size_` rather than
  `style_.preferred_size`.
- `.gap(float)` can share the retained style path because row, column, and
  vertical stack builders already copy `style_.gap` into their concrete element
  before appending children.

## 2026-06-30 Remaining Steps 93-128 Plan Shape

- The user-facing "后 40 步" still refers to the Step 89-128 target band, but
  Steps 89-92 are already complete on `master`; the active implementation
  queue is therefore Steps 93-128.
- The safest order is still mostly sequential because Step 93-98 authoring and
  render entry unlock model/app lifecycle work, Step 99-108 lifecycle unlocks
  child-view ancestry, and Step 109-118 interaction/layout metadata feeds the
  Step 119-128 renderer/platform/demo hardening work.
- Step 124-127 are the highest platform-risk region because they cross from
  pure command/runtime metadata into Win32 and Wayland backend behavior; keep
  memory clipboard and deterministic tests available while platform backends
  mature.

## 2026-06-30 Style Unit And Color Helpers

- `px(float)` can remain a direct `float` identity helper for now because every
  current style/layout field already stores pixel-like lengths as `float`.
- Free `edges(...)` helpers should forward to the existing `EdgeSizes::all`,
  `EdgeSizes::axes`, and `EdgeSizes::trbl` constructors so Step 93 builder
  shortcuts can use concise authoring names without adding another edge type.
- `rgb` and `rgba` normalize integer 0-255 color channels into the existing
  floating `Color` representation; alpha remains a `float` so authored opacity
  composes with the current `Color::a` field directly.

## 2026-06-30 Steps 89-128 Execution Planning

- The remaining Windows/Linux core API work is best sequenced in four bands:
  authoring/render entry first, then model/app/window lifecycle, then
  event/layout depth, then renderer/text/platform hardening and the public demo.
- Step 92 should stay deliberately small: inline `px`, `rgb`, `rgba`, and
  `edges(...)` helpers can layer over the existing `float`, `Color`, and
  `EdgeSizes` primitives without adding a new length type yet.
- Step 93 can then mutate the builder's retained `Style` directly, because
  `ElementBuilder` already owns `style_` and builds `StyledElement`, flex,
  stack, and text variants from that data.
- Steps 96-98 should preserve the existing `View::paint(...)` contract while
  adding `View::render(ViewContext&)`; this lets current tests and demos keep
  compiling while the GPUI-like render path becomes available.
- Before implementing Step 92, the Step 91 feature worktree still needs the
  post-merge WSL verification/cleanup on `master`.

## 2026-06-30 Element Child Overloads

- `AnyElement` is already `std::unique_ptr<Element>`, so the existing
  base-ownership overload is the public `AnyElement` path; Step 91 only needs
  extra overloads for builder materialization and typed derived ownership.
- `.child(ElementBuilder)` should call `into_element(...)` internally, keeping
  all builder-to-owned conversion behavior in the same public convention added
  in Step 89.
- The typed ownership template should exclude `Element` itself so normal
  `std::unique_ptr<Element>` and `AnyElement` calls continue using the existing
  non-template overload and overload resolution stays predictable.

## 2026-06-30 Element Authoring Factories

- Free factories should return `ElementBuilder`, not `AnyElement`, because the
  existing fluent builder methods still need to compose before ownership is
  materialized through `into_element(...)` or `build()`.
- `div()` maps directly to the current styled-box builder, `h_flex()` and
  `v_flex()` map to row/column flex builders, `v_stack()` maps to the stack
  builder, and `text(TextModel&)` preserves the existing non-owning text-model
  binding convention.
- Keeping these helpers in `element.hpp` makes them visible through both
  `cgpui/ui/element.hpp` and the public prelude without creating another
  authoring header before the child-conversion overloads land.

## 2026-06-30 AnyElement Authoring

- `AnyElement` can be a public alias for the existing
  `std::unique_ptr<Element>` ownership model, which keeps Step 89 source
  compatible with all current tree, builder, and runtime installation APIs.
- `into_element(ElementBuilder)` should build by value from the existing
  rvalue-qualified `ElementBuilder::build()` path; this establishes the common
  conversion convention without adding Step 90 factory helpers yet.
- Keeping `into_element(AnyElement)` as the owned-element identity overload
  gives later child overloads and free factories one public spell to target
  while avoiding reference-counted ownership or type erasure churn.

## 2026-06-30 128-Step Planning

- The next highest-leverage path toward a GPUI-like core API is authoring
  ergonomics first: `AnyElement`, free element factories, child conversion, and
  fluent style shortcuts make later demos and tests express the intended public
  surface instead of the current lower-level builder plumbing.
- View/model/app lifecycle should follow once authoring helpers exist, because
  `View::render(ViewContext&)`, model update invalidation, and app window
  opening all need a stable element-returning convention.
- Renderer and platform work should remain command- and abstraction-driven:
  add paint/text/clip/cursor commands at the UI/runtime boundary before
  teaching Vulkan, Win32, or Wayland to consume them.
- Full macOS parity is still not part of Steps 89-128; each new API should stay
  Mac-neutral and avoid introducing fresh Vulkan/Win32/Wayland assumptions into
  public core headers.

## 2026-06-30 Mac Readiness Audit

- The repository already has macOS source slots:
  `src/platform/macos/macos_application.mm` creates a Cocoa `NSWindow` backed
  by `CAMetalLayer`, and `src/renderer/metal/metal_renderer.mm` accepts
  `MetalSurfaceHandle`.
- `xmake.lua` already gates macOS platform and Metal renderer sources under
  `is_plat("macosx")`, so Step 88 does not need to move files or alter
  Windows/Linux build branches.
- The useful boundary improvement is a small public target map:
  Windows/Win32 -> Vulkan, Linux/Wayland -> Vulkan, and macOS/Cocoa -> Metal.
- Full Mac parity is still intentionally out of scope for this slice; the
  next Windows/Linux API steps should stay renderer-neutral above the existing
  `Renderer` and `NativeSurfaceHandle` abstractions.

## 2026-06-29 Baseline

- Current UI runtime has a single root `View`, a stable root `ViewId{1}`, input snapshots, keyboard focus owner, and pointer capture owner.
- `View::handle_event` currently returns `void`; changing this to an event result is the next routing prerequisite.
- Existing tests are concentrated in `tests/ui/window_runtime_test.cpp` and can verify runtime behavior without real platform windows.
- The main worktree has a pre-existing untracked `.vscode/` directory that should remain untouched.

## 2026-06-29 Event Dispatch Observability

- A dispatch observation can stay entirely in the UI runtime layer: no Win32,
  Wayland, or Vulkan platform changes are needed.
- Runtime lifecycle events handled internally by `WindowRuntime` should not
  produce view dispatch records.
- `WindowRuntimeContext::last_event_dispatch` intentionally reports the
  previous view-dispatched event while a view is handling the next event; the
  after-event callback sees the current record after it has been stored.

## 2026-06-29 Event Routing Shell

- A root-only router is enough for the current single-view runtime while
  establishing the API shape that later element/view routing can replace.
- `WindowRuntimeContext::event_route` is current during `View::handle_event`,
  while `last_event_dispatch` remains the previous completed dispatch until the
  view returns.
- `EventDispatchRecord` now duplicates `view_id`/`event_kind` as compatibility
  fields and stores the full `EventRoute` for future routing expansion.

## 2026-06-29 Entity Store Skeleton

- The entity store can live in `cgpui/core/entity.hpp` as a header-only
  template without adding a source file to `cgpui_core`.
- Monotonic, non-reused ids keep the first skeleton simple and avoid stale-id
  aliasing until a future generation/index scheme is justified.
- A typed `EntityId<T>` gives compile-time separation between stores while
  preserving a simple numeric `.value` for tests and diagnostics.

## 2026-06-30 Context Entity Access

- `WindowRuntimeContext` is acting as a capability handle: it can stay `const`
  while still forwarding controlled mutations into runtime-owned state.
- `WindowRuntime` can hold per-type entity stores behind
  `std::unordered_map<std::type_index, std::any>` without adding new source
  files or platform-specific code.
- Missing or removed entities should stay soft-fail APIs for now:
  read/mutate return `nullptr`, and remove returns `false`.

## 2026-06-30 View Identity Allocation

- Step 6 only needs identity allocation, not a view tree or reconcile pass.
- Keeping `ViewId{0}` invalid, root at `ViewId{1}`, and runtime allocation
  from `ViewId{2}` gives later element/view storage a simple stable id source.
- A runtime-local monotonic counter is enough until later steps add removal,
  reconciliation, or generation/index semantics.

## 2026-06-30 Element Base API

- `ElementId` should stay separate from `ViewId`; element tree ids and view
  routing ids are related later, but they are not the same type at this layer.
- `Element` should not allocate its own ids. The next tree/reconcile steps can
  assign ids through `assign_id()` and preserve them across rebuilds.
- Keeping the base class free of layout/event methods avoids coupling Step 7 to
  the later layout and routing slices.

## 2026-06-30 Element Tree Container

- `ElementTree` can remain header-only for now because it stores generic
  `Element` ownership and simple relationship vectors.
- Replacing the root clears existing node relationships but keeps monotonic id
  allocation moving forward, which avoids stale-id aliasing.
- The container intentionally rejects unknown parents softly with
  `ElementId{0}`; richer diagnostics can wait until tree construction APIs are
  more expressive.

## 2026-06-30 Element Reconcile Pass

- A parent-local index reconcile is enough for the first pass because Step 8
  already records ordered children.
- Reconcile should replace element instances while preserving their assigned
  ids; this gives later layout/routing steps stable ids across simple rebuilds.
- Keyed diffing, pruning, and type-aware compatibility checks are intentionally
  outside this slice.

## 2026-06-30 Layout Primitives

- Layout primitives can live in a separate `cgpui/ui/layout.hpp` header so the
  element tree does not grow layout responsibilities before Step 11.
- Using zero min constraints and infinite max constraints gives unconstrained
  layout callers a simple default while preserving explicit min/max tests.
- `constrain_size` is intentionally only a per-axis clamp helper; element layout
  methods, bounds storage, and traversal remain later slices.

## 2026-06-30 Fixed Size Layout

- Adding layout as a virtual method on `Element` gives later stack and routing
  steps a common leaf/container contract without adding tree traversal yet.
- The base element layout returns constrained zero size, which keeps existing
  test-only elements usable while giving constraints a visible effect.
- `FixedSizeElement` is deliberately leaf-only: it stores a preferred size and
  delegates all min/max behavior to `constrain_size`; bounds persistence and
  child layout remain future slices.

## 2026-06-30 Vertical Stack Layout

- A self-contained `VerticalStackElement` is enough for Step 12; it avoids
  coupling layout traversal to `ElementTree` before bounds and hit testing
  exist.
- The first stack pass can lay out children with default unconstrained
  `LayoutInput` and only clamp the final stack size; constraint propagation can
  become a later refinement when richer layout rules exist.
- The stack intentionally does not persist child origins or bounds yet, because
  Step 13 is the first slice that needs retained layout geometry.

## 2026-06-30 Hit Testing

- Retained bounds can stay on `Element` as mutable state updated by const
  `layout()` calls, matching the existing const layout API without adding a
  renderer or runtime dependency.
- `hit_test(Point)` should soft-fail with `ElementId{0}` before layout or
  outside the retained bounds, which keeps Step 14 pointer routing simple.
- `VerticalStackElement` can record direct child bounds while laying out owned
  children and use child-first hit testing; richer nested coordinate propagation
  can wait until element-tree traversal or more complete layout context exists.

## 2026-06-30 Pointer Hit Routing

- The existing event routing shell can carry an optional `ElementId` without
  changing root-view delivery yet.
- A non-owning runtime element root is enough for this slice because ownership,
  reconcile, and layout scheduling are still outside the runtime.
- Pointer capture should not be folded into hit routing yet; Step 15 can decide
  when capture should override a fresh hit-test result.

## 2026-06-30 Pointer Capture Routing

- Element-level capture can coexist with the older root-view capture state by
  storing a separate optional `ElementId` owner.
- Capture should override fresh hit testing only for pointer events, preserving
  non-pointer routes.
- Owner-matched release keeps the API soft-fail and mirrors the existing
  view-owner release behavior.

## 2026-06-30 Keyboard Focus Routing

- Element-level keyboard focus can reuse `EventRoute::target_element_id`, just
  like pointer routing, because the route already represents the event's
  intended element target.
- Keyboard and text-input events should use the focused element owner before
  any pointer-specific routing logic; pointer hit testing and pointer capture
  remain unaffected.
- Keeping view-level and element-level focus owners as separate optional fields
  preserves old root-view focus tests while exposing the element owner needed
  for later key binding and text-input slices.

## 2026-06-30 Action Command Primitive

- A runtime-local action handler table is enough for the first primitive and
  avoids a global registry before key binding or view trees exist.
- Reusing `EventResult` for action handlers keeps consumed/cancelled semantics
  aligned with event handling.
- Recording the last `ActionDispatchResult` gives Step 18 a simple observable
  target when key bindings start dispatching named actions.

## 2026-06-30 Key Binding Table

- A runtime-local vector of bindings is enough for exact key/modifier matching
  in the first slice; scoped binding tables and priorities can wait.
- Dispatching the matched action before root view event handling gives action
  handlers access to the current `EventRoute` while preserving existing view
  event delivery.
- Matching only `KeyboardKey` events keeps text input independent until the text
  model and focused-text routing slices land.

## 2026-06-30 Text Model

- A byte-offset cursor keeps the first text model small while still allowing
  UTF-8-safe codepoint navigation by skipping continuation bytes.
- The source test should stay ASCII on Windows; UTF-8 examples can be expressed
  with byte escapes to avoid MSVC code-page warnings.
- `backspace()` deletes the codepoint before the cursor; the test originally
  moved the cursor to offset 0 before expecting a successful second backspace,
  which was a test expectation bug rather than an implementation bug.

## 2026-06-30 Text Input Routing

- Binding text models by `ElementId` lets the Step 16 keyboard-focus route
  become actionable without introducing element event handlers yet.
- Runtime-owned text model pointers should remain non-owning in this slice; the
  owning element/view layer does not exist yet.
- The text-input routing test must fire the third key event before expecting
  the existing helper to release element keyboard focus; otherwise the second
  text input correctly still targets the model.

## 2026-06-30 40-Step Milestone

- Step 21 should introduce style as inert data first, not layout or painting
  behavior, so later builder, layout, and paint-tree steps have a stable shared
  vocabulary.
- Style primitives can stay header-only in `include/cgpui/ui/style.hpp` and
  reuse `Color` from `core/geometry.hpp` to avoid duplicating color types.
- A single `EdgeSizes` structure is enough for padding/border-width symmetry
  and keeps Step 24 padding layout and Step 25 border primitives aligned.

## 2026-06-30 Element Builder API

- `StyledElement` can be introduced as an inert wrapper first: it owns style
  data and an optional child, but does not paint backgrounds or apply padding
  until the dedicated Step 23 and Step 24 slices.
- Keeping `ElementBuilder::build()` returning `std::unique_ptr<Element>` matches
  the existing tree/container APIs and avoids introducing reference-counted
  element ownership prematurely.

## 2026-06-30 Styled Background Paint

- Element-level painting can reuse the existing `PaintList` command vocabulary
  without changing renderer or platform code.
- Keeping `Element::paint(PaintList&)` as a no-op default preserves existing
  test elements while letting styled/container elements opt into paint output.
- `StyledElement` should emit its background before painting its child, matching
  normal parent-background-then-content ordering and setting up Step 26 paint
  tree traversal.

## 2026-06-30 Styled Padding Layout

- Padding can be handled inside `StyledElement::layout` before a separate box
  model abstraction exists: content size comes from the child or preferred size,
  then top/right/bottom/left padding expands the outer size.
- Child bounds should be rewritten after the child layout pass so hit testing
  and later paint traversal observe the padded child origin.
- Existing layout constraints in this codebase clamp container output after
  child measurement, so this slice keeps the same simple constraint behavior.

## 2026-06-30 Border Style Primitives

- Border color and radii can be introduced as inert `Style` data without
  touching layout or paint yet; Step 26/27 can decide how much of the box model
  to materialize in paint commands.
- `BorderRadii` mirrors `EdgeSizes` as a compact POD helper, keeping top-left,
  top-right, bottom-right, and bottom-left explicit for later rounded-rect work.
- `Style::border_width` already existed from Step 21, so this slice only needed
  color and radius fields plus fluent setters.

## 2026-06-30 Element Paint Tree

- `ElementTree::paint(PaintList&)` can be a thin recursive traversal over the
  existing parent/child relationship data; no renderer or platform changes are
  needed for this slice.
- Painting an element before its children preserves the Step 23 parent
  background before content ordering.
- Sibling paint order follows the stored child id vector, matching append and
  reconcile order until a later z-order primitive changes that policy.

## 2026-06-30 Clip Overflow Primitives

- Clip and overflow can start as inert `Style` data; applying clip stacks to
  paint traversal is a later renderer/painter concern.
- `Overflow::visible` as the default preserves all existing layout, hit-test,
  and paint behavior until explicit hidden overflow is requested.
- Keeping `clip_rect` optional distinguishes "no authored clip" from an
  authored zero-size clip rectangle, which will matter when paint traversal
  starts honoring clipping.

## 2026-06-30 Z Order Paint Order

- `Style::z_index` is the smallest API surface for authored paint ordering
  because `StyledElement` already owns style data and `ElementTree::paint`
  already controls sibling traversal.
- Sorting only same-parent children keeps the existing parent-before-children
  invariant from Step 26 while making sibling paint order deterministic.
- `std::stable_sort` preserves insertion/reconcile order for equal z-index
  siblings, so existing append-order paint tests remain meaningful.

## 2026-06-30 Hover State Tracking

- Hover can be modeled as input state first: no hover enter/leave event types
  are needed until element handlers exist.
- Pointer capture should override event routing, not the physical hover target;
  the runtime therefore computes hover from the live pointer hit test before
  applying capture to `EventRoute`.
- Clearing hover on pointer-move misses keeps stale element ids from surviving
  after the pointer leaves the current element root.

## 2026-06-30 Cursor Shape State

- Cursor shape can start as runtime-visible input state before platform cursor
  handles are wired into Win32 and Wayland.
- Binding cursor shapes by `ElementId` composes directly with the Step 29 hover
  state and avoids inventing element event handlers in this slice.
- Restoring `CursorShape::default_arrow` on hover misses prevents stale cursor
  state from outliving the hovered element.

## 2026-06-30 Scroll State Model

- The first scroll primitive can remain UI-layer and header-only: it only needs
  viewport size, content size, and a clamped offset.
- Offsets should reclamp whenever viewport or content size changes so stale
  scroll positions cannot survive after content shrinks.
- Axis scrollability is derived from positive max offset, not from the current
  offset, which keeps `can_scroll_x`/`can_scroll_y` useful at both ends of a
  scroll range.

## 2026-06-30 Flex Layout Basics

- `FlexElement` can start as a sibling to `VerticalStackElement` rather than a
  replacement: it keeps row/column semantics explicit without disturbing the
  existing vertical-stack tests.
- The first flex slice should only measure natural child sizes and clamp the
  container output; grow, shrink, gap, align, and justify behavior can remain
  future work.
- Reusing the same child-first hit testing pattern as `VerticalStackElement`
  keeps routed pointer behavior consistent once flex elements are installed as
  runtime element roots.

## 2026-06-30 Runtime Invalidation

- Runtime invalidation can start as observable dirty state without scheduling
  redraws yet; Step 34 can connect dirty state to platform redraw requests.
- A layout invalidation should imply paint invalidation because a layout pass
  changes paint bounds, while a paint invalidation alone should not imply
  layout work.
- Clearing invalidation explicitly keeps tests and future frame processing able
  to model "dirty work consumed" separately from "new work requested".

## 2026-06-30 Update Scheduling

- Invalidation-triggered redraw should be deferred until after view event and
  after-event callbacks finish; the test fake window synchronously emits redraw
  events, so immediate requests would reenter the runtime too early.
- A single `redraw_scheduled_` flag is enough to coalesce layout and paint
  requests made during the same event into one platform redraw request.
- Successful redraw should clear both invalidation and scheduling state so
  later model/view changes can schedule the next frame cleanly.

## 2026-06-30 View Model Subscriptions

- A first subscription skeleton can be type-erased with `std::type_index` plus
  the typed `EntityId<T>::value`; this keeps the API typed while avoiding a
  full dependency graph.
- Querying subscriptions by `ViewId` is enough for tests and future debugging;
  notification can scan the small vector until a richer storage model is
  needed.
- Entity-change notification should request layout rather than paint only,
  because model changes may affect both layout and paint output.

## 2026-06-30 Text Selection Range

- Text selection can live entirely in `TextModel` as byte offsets for this
  slice, matching the existing byte-offset cursor while preserving UTF-8-safe
  movement helpers.
- Storing anchor and head separately gives Step 37 enough state for
  Shift+movement behavior, while `selection()` exposes a normalized start/end
  range for edit operations.
- Plain insertion should replace a non-collapsed selection and then collapse to
  the inserted text end, matching typical text editing behavior without adding
  platform clipboard or IME coupling yet.

## 2026-06-30 Text Edit Actions

- A small `TextEditAction` enum keeps keyboard editing semantics in the
  platform-neutral text model before binding them to Win32 or Wayland key
  events.
- Selection extension should preserve the original anchor and move only the
  head/cursor, which gives later Shift+Arrow routing the right primitive.
- `backspace()` and `delete_forward()` should erase a non-collapsed selection
  before falling back to adjacent-codepoint deletion, so direct text-model calls
  and edit-action dispatch behave consistently.

## 2026-06-30 Clipboard Abstraction

- The first clipboard slice can be a platform-layer text contract, independent
  from `PlatformApplication`, so the API is usable in tests and can later be
  backed by Win32 or Wayland system clipboard protocols.
- A shared `MemoryClipboard` implementation is enough for the Step 38 skeleton;
  platform-specific system integration can replace the factory implementation
  without changing callers.
- Keeping the factory in `cgpui_platform` instead of the Win32/Wayland leaf
  targets avoids duplicate definitions when examples link both shared platform
  code and a platform-specific implementation.

## 2026-06-30 IME Composition Skeleton

- IME composition should be a separate platform event from `TextInput` because
  update/preedit text is transient while commit text mutates the text buffer.
- Composition events should use the same keyboard-focus element route as key
  and text-input events; this lets Win32 and Wayland feed future IME messages
  into the focused text model without inventing a parallel routing path.
- The first text-model composition state can stay as one optional UTF-8 string:
  update replaces the preedit text, commit inserts it through normal text
  insertion, and cancel clears it without changing the underlying buffer.

## 2026-06-30 New API Demo

- The demo can exercise the new API surface without waiting for text shaping:
  it paints element rects, a cursor rect derived from `TextModel::cursor`, and a
  composition underline derived from the IME composition state.
- Runtime installation can stay non-owning for Step 40 by passing the demo's
  owned root element to `WindowRuntime::set_element_root`; later runtime-owned
  element-tree steps can replace that with a safer ownership model.
- `CGPUI_DEMO_INJECT_TEXT` gives the demo a scripted state-change path for
  smoke tests while keeping real interactive behavior available in normal runs.

## 2026-06-30 ElementTree Root Layout

- `ElementTree::layout_root` is intentionally a root helper, not a full tree
  traversal: existing container elements still own their child layout behavior.
- Empty-tree layout should return constrained zero size, matching base element
  layout semantics and giving callers a safe no-root path.

## 2026-06-30 ElementTree Root Hit Testing

- `ElementTree::hit_test_root` mirrors `layout_root`: it is a safe root-level
  convenience, while element/container implementations still define hit-test
  traversal and child priority.
- Empty-tree hit testing should return `ElementId{0}` so runtime routing can
  distinguish "no element root" from a valid target.

## 2026-06-30 Runtime-Owned Element Tree

- Runtime can support both the old non-owning `set_element_root` path and the
  new owning `set_element_tree` path by making them mutually exclusive; this
  avoids stale pointers when ownership changes.
- Pointer routing should use `ElementTree::hit_test_root` when an owned tree is
  installed, preserving the legacy `Element::hit_test` path for external roots.

## 2026-06-30 Runtime-Owned Tree Layout

- Redraw is the first safe place to auto-layout an owned element tree because
  the runtime has current framebuffer/viewport size and can update retained
  bounds before later pointer events rely on hit testing.
- Using viewport size as the max layout constraint preserves existing element
  min defaults while preventing oversized preferred roots from exceeding the
  drawable area.

## 2026-06-30 Element Event Hook

- `EventResult` belongs in the element-visible API once elements can handle
  events; keeping `ui.hpp` as a consumer avoids duplicating result semantics.
- The first `ElementEventContext` only needs the routed target element id. View
  runtime/application access can be layered later through a richer context.
- `StyledElement` forwarding to its child preserves wrapper composition until
  styled boxes grow their own interaction behavior.

## 2026-06-30 Runtime Element Event Dispatch

- Runtime event dispatch can treat element handling as the first phase and the
  existing root view as fallback, preserving old view behavior for unhandled
  element events.
- Owned `ElementTree` can resolve arbitrary routed ids with `get(id)`, while
  the legacy non-owning root can only safely dispatch to the root id itself.
- `EventDispatchRecord::result` should represent the final handler result,
  whether that result came from an element or from the fallback view.

## 2026-06-30 Element Builder Flex Helpers

- `ElementBuilder` can grow a small kind discriminator without changing the
  existing `box()` call sites; `box()` still builds a `StyledElement`.
- Row and column helpers should build concrete `FlexElement` containers so
  existing flex layout, hit testing, and future gap support remain centralized.
- Keeping box as a single-child wrapper preserves the earlier styled-box API
  while allowing row/column helpers to accept multiple chained children.

## 2026-06-30 Element Builder Stack Helper

- The same builder kind discriminator used for flex helpers can cover
  `v_stack()` without changing `VerticalStackElement` itself.
- Building a concrete `VerticalStackElement` keeps older stack-specific tests
  and future stack-only gap behavior independent from generic flex containers.
- The builder's accumulated child vector is now the shared path for multi-child
  container helpers.

## 2026-06-30 Element Builder Fixed-Size Helper

- `fixed_size(Size)` fits the existing builder kind model as a leaf builder,
  not a styled box variant.
- Building a concrete `FixedSizeElement` preserves the existing constraint and
  retained-bounds behavior without adding a second preferred-size path.
- The helper intentionally ignores accumulated children because fixed-size
  elements are leaf nodes in the current element model.

## 2026-06-30 Gap Style Layout

- `Style::gap` can stay as a single main-axis spacing primitive for now:
  vertical stack and flex column apply it on Y, while flex row applies it on X.
- Applying gap only when `child_index > 0` preserves existing zero-gap layout
  and avoids adding trailing space after the final child.
- The existing builder `style(...)` hook is enough to author container gap for
  `row`, `column`, and `v_stack` without wrapping those multi-child containers
  inside an extra `StyledElement`.

## 2026-06-30 Margin Style Layout

- Reusing `EdgeSizes` for `Style::margin` keeps the box-model vocabulary aligned
  with padding and border width without adding another edge type.
- Margin is modeled as outer size in `StyledElement::layout`; existing no-margin
  tests preserve prior behavior because the default edge values are zero.
- Child layout bounds should include the margin offset before padding so later
  hit testing and paint commands see content positioned inside the outer box.

## 2026-06-30 Border Paint Emission

- Border rendering can reuse the current `SolidRect` paint command vocabulary:
  styled borders are four filled rects emitted after background paint.
- Requiring `border_color` before emitting border rects preserves the previous
  inert behavior for authored widths that do not yet specify a color.
- Keeping side rects between top and bottom avoids double-painting corners while
  still giving Vulkan/Wayland and Win32 the same command stream shape.

## 2026-06-30 Overflow Clip Metadata

- Clip handling can start as paint-command metadata: renderers can continue
  drawing `SolidRect` unchanged while future renderer slices learn to honor the
  optional `PaintCommand::clip_rect`.
- A simple `PaintList` clip stack is enough for nested styled elements and keeps
  clip state out of individual element implementations beyond push/pop scope.
- Hidden overflow should use the element's retained layout bounds by default,
  while authored `Style::clip_rect` can override the metadata rect when present.

## 2026-06-30 Text Element Skeleton

- The first `TextElement` can stay in the element layer as a non-owning
  `TextModel*` binding; runtime-owned text-model routing already uses the same
  lifetime convention.
- A fixed 8x16 skeleton metric is enough to make layout, hit testing, and paint
  observable without introducing shaping, fonts, glyph atlases, or renderer text
  commands in this slice.
- Paint can emit a placeholder `SolidRect` through the existing paint path so
  Win32/Vulkan and Wayland/Vulkan remain unchanged until a later text renderer
  command is justified.

## 2026-06-30 Runtime Text Edit Actions

- Runtime text edit routing can mirror key bindings as a platform-neutral
  binding table from `KeyboardKey` data to `TextEditAction`, avoiding hard-coded
  Win32 or Wayland key assumptions in the text model.
- The existing element keyboard-focus owner is the right target selector:
  keyboard events already route to it, and text input/IME routing already look
  up text models by that focused `ElementId`.
- Matching edit bindings before view fallback keeps the model updated before
  observers inspect the event, while leaving unbound keys and missing text
  models as no-op cases for compatibility.

## 2026-06-30 Runtime Clipboard Paste

- Runtime clipboard paste can use the existing platform-neutral `Clipboard`
  interface without introducing Win32 or Wayland system clipboard details into
  the UI layer.
- Keeping the clipboard pointer non-owning matches the runtime's current
  non-owning `TextModel*` binding convention and lets tests inject
  `MemoryClipboard` directly.
- Paste should target the focused element's bound `TextModel`, so it composes
  with the same keyboard-focus route used by text input, IME, and edit actions.

## 2026-06-30 Runtime Clipboard Copy

- Copying selection text should live on top of `TextModel` selection state
  rather than duplicating byte-range normalization in the runtime.
- The runtime clipboard copy path can share the same non-owning `Clipboard*`
  and focused-element text-model lookup introduced for paste.
- Collapsed selections should not overwrite clipboard contents; returning
  `false` gives callers a simple observable no-op.

## 2026-06-30 Runtime Clipboard Cut

- Cut can compose copy plus the existing selection-aware
  `TextModel::delete_forward`, which avoids adding a second selected-range
  deletion primitive in the runtime.
- The operation should only mutate text after clipboard write succeeds, so
  failed copy/no selection/no focused model leaves the buffer untouched.
- After deletion, existing `TextModel` behavior collapses the cursor to the
  removed selection start, matching the text-edit action path.

## 2026-06-30 ViewContext Convenience

- A type alias keeps the public `ViewContext` spelling GPUI-like without
  duplicating runtime context storage or breaking existing
  `WindowRuntimeContext` callbacks.
- Thin context methods are enough for common authoring paths: they forward to
  the existing runtime APIs and preserve all focus, capture, clipboard, and
  invalidation behavior.
- Invalidation convenience tests should observe state during view event
  handling, not after-event callbacks, because deferred redraw can flush and
  clear invalidation before `after_event` returns.

## 2026-06-30 App Runner Skeleton

- The first public runner can sit above `WindowRuntime` rather than replacing
  it: the runner owns the renderer, while `WindowRuntime` still owns event
  dispatch and redraw behavior.
- A setup callback is enough for this slice to let callers bind elements,
  models, actions, cursors, and callbacks before the platform event loop starts.
- Accepting an injected `PlatformApplication&` and renderer factory keeps the
  runner deterministic in tests and leaves default platform/renderer creation
  for a later convenience overload.

## 2026-06-30 Text Element Builder

- `ElementBuilder::text(TextModel&)` should mirror `TextElement`'s non-owning
  binding instead of copying text, so runtime text-model bindings and builder
  authored text share the same lifetime convention.
- Building text as a direct leaf keeps the initial API small; styling can stay
  in wrapper elements until text-specific font, color, and shaping primitives
  exist.

## 2026-06-30 Focusable Element Hook

- Focus activation belongs first at the element boundary: runtime click routing
  can query `Element::focusable()` and call `Element::focus(...)` without
  baking widget-specific behavior into `WindowRuntime`.
- Keeping the default element non-focusable preserves existing hit-test and
  event-routing behavior for passive layout and paint elements.

## 2026-06-30 Runtime Click Focus

- Runtime focus activation can reuse the already-computed event route: after
  pointer capture/hit-test routing chooses a target element, a left-button press
  is enough to request element keyboard focus and call its focus hook.
- Calling `focus(...)` before normal element event dispatch lets the current
  pointer event and the following keyboard event observe the updated focus
  owner through the existing `WindowRuntimeContext` shape.

## 2026-06-30 Element Enabled State

- Enabled/disabled state fits best on `Element` rather than `Style`: it is an
  interaction primitive first, and Step 65 can use the same base API to skip
  dispatch without inventing style-aware event routing.
- Builder-level `enabled(bool)` should apply to every concrete element kind so
  disabled state stays orthogonal to layout choice, text leaves, and styled
  wrappers.

## 2026-06-30 Disabled Element Event Gate

- Disabled state is now a routing/interaction gate rather than a hit-test gate:
  route metadata can still identify the hit element, but the disabled element's
  handler is skipped.
- View fallback still observes the original event route after a disabled routed
  element is skipped, which preserves existing dispatch diagnostics and lets
  parent/view-level behavior decide whether to consume the event.
- `StyledElement` treats a disabled child the same as no interactive child for
  event forwarding, returning unhandled without mutating child event state.

## 2026-06-30 ElementTree Preorder Traversal

- `ElementTree::preorder_ids()` should be structural, root first and then each
  stored child subtree in child-list order; it intentionally does not reuse
  paint order, which may sort siblings by z-index.
- Returning ids rather than element pointers keeps the helper simple,
  copyable, and compatible with later APIs that can call `get(id)` or filter by
  id without exposing internal nodes.
- Empty-tree traversal returns an empty vector, matching the existing soft-fail
  behavior of root layout, hit testing, and paint traversal.

## 2026-06-30 ElementTree Typed Lookup

- `ElementTree::find_as<T>(ElementId)` is a convenience over the existing
  id-based `get` contract, not a new ownership path; it returns null for
  missing ids and type mismatches.
- Providing both const and mutable overloads lets tests and future ViewContext
  helpers preserve const-correctness while avoiding repeated `dynamic_cast`
  boilerplate at call sites.

## 2026-06-30 ViewContext Text Model Binding

- ViewContext text binding is just a capability-forwarding helper over runtime
  text-model binding; it should not introduce a second ownership or lookup path.
- Binding during an event should be immediately visible to subsequent text input
  routed to the focused element, matching the existing runtime text input path.

## 2026-06-30 ViewContext Element Tree Installation

- ViewContext element tree installation should forward ownership directly into
  `WindowRuntime::set_element_tree`; the context remains a capability wrapper,
  not a second tree store.
- Installing a tree during an event composes with `request_layout()`: the
  deferred redraw lays out the owned tree before later pointer routing uses it.

## 2026-06-30 ViewContext Action Helpers

- Action registration and dispatch can live on `ViewContext` as thin forwards
  over the runtime action table, preserving the existing handler storage and
  last-dispatch observability.
- `ActionHandler` can be declared before `WindowRuntimeContext` is defined
  because it only needs the context by reference; this lets context methods use
  the public handler alias without adding another callback type.

## 2026-06-30 ViewContext Key Binding Helper

- Key binding through `ViewContext` can reuse the runtime binding table without
  changing keyboard dispatch order; a binding registered during one event is
  available to subsequent keyboard events.
- Keeping action registration and key binding as separate context helpers
  mirrors the runtime API and lets later authoring helpers compose them without
  adding a combined shortcut abstraction too early.

## 2026-06-30 ViewContext Text Edit Binding Helper

- Text-edit bindings registered from `ViewContext` can stay as direct forwards
  to the runtime binding table; current-key dispatch still happens before the
  view handler, so tests should assert effects on later key events.
- `TextModel(std::string)` already places the cursor at the end, which is enough
  for text-edit binding tests without adding a cursor-positioning helper.

## 2026-06-30 ViewContext Cursor Binding Helper

- Cursor binding through `ViewContext` should stay a thin capability forwarder
  over `WindowRuntime::set_element_cursor`; the runtime remains the single
  storage owner for element-to-cursor mappings.
- A cursor binding registered during one event is visible to later pointer
  hover routing, matching the existing runtime cursor state path and preserving
  platform-neutral behavior for Windows and Wayland.

## 2026-06-30 ViewContext Element Focus Helpers

- Short `ViewContext::focus(ElementId)` and `blur(ElementId)` helpers can layer
  over the existing element keyboard-focus request/release APIs without changing
  routing semantics.
- Keeping the long `request_keyboard_focus(ElementId)` APIs alongside the short
  helpers preserves compatibility while giving author code a more GPUI-like
  spelling for common element focus flows.

## 2026-06-30 ViewContext Pointer Capture Element Helpers

- Element pointer capture should be authorable with an `ElementId` overload on
  `ViewContext`, while the runtime continues storing the existing
  `PointerCaptureOwner` tagged owner.
- The overloads are intentionally additive: existing `PointerCaptureOwner`
  calls remain available for view-level capture and explicit owner matching.

## 2026-06-30 Element Builder Click Handler

- Builder-level click handling can be implemented as an opt-in wrapper element,
  preserving existing dynamic-cast expectations for elements built without a
  click handler.
- The wrapper should preserve child layout, paint, and hit-test behavior, while
  consuming pointer-press handling through the attached handler before falling
  back to the child for other events.
- Wrapper elements created by builder helpers must mirror the builder's
  enabled state on the wrapper itself, not only on the wrapped child, because
  direct event dispatch sees the outer element first.

## 2026-06-30 Element Builder Focusable Helper

- Builder-level focusability can also be an opt-in wrapper, preserving existing
  concrete element types unless the helper is requested.
- Wrapper composition order matters: behavior wrappers like click handling need
  to stay inside capability wrappers like focusability so `focusable()` remains
  visible on the built element while event handling still reaches the click
  handler.

## 2026-06-30 Element Builder Key Handler

- Element-level key handling can use the same opt-in wrapper pattern as click
  handling, but it should accept the concrete `KeyboardKey` alongside the
  element event context so author code does not need to inspect the variant.
- Key and click handlers should remain behavior wrappers inside focusability,
  preserving `focusable()` visibility when helpers are composed.

## 2026-06-30 Element Builder Disabled Helper

- `ElementBuilder::disabled()` is pure authoring sugar over `enabled(false)`;
  keeping it as a flag flip lets the existing wrapper enabled propagation
  handle composed elements consistently.

## 2026-06-30 ElementTree Enabled Traversal

- `enabled_preorder_ids()` filters by each node's current `enabled()` state at
  push time but still traverses descendants, which makes it a general enabled
  query rather than a disabled-subtree pruning policy.

## 2026-06-30 Disabled Focus Activation

- Disabled elements remain routable and observable through hit testing, but
  runtime click-to-focus activation should use the same interaction gate as
  element event dispatch: the element must be both enabled and focusable.

## 2026-06-30 Disabled Hover Cursor

- Hover state should continue to describe the physical hit target, but cursor
  shape is an interaction affordance and should only be driven by enabled
  elements when the runtime can resolve the hovered id to a live element.
- Legacy non-owning roots can hit-test child ids that `routed_element()` cannot
  safely resolve, so cursor lookup keeps the old behavior for unresolved ids
  and only suppresses cursor bindings for resolved disabled elements.

## 2026-06-30 Focused Text Model Lookup

- The runtime already treats element keyboard focus as the owner selector for
  text input, IME, text edit actions, and clipboard operations, so a public
  `focused_text_model()` helper can be the single soft-fail lookup for that
  binding.
- Clipboard paste/copy/cut can reuse the helper directly, reducing duplicate
  `keyboard_focus_element_owner_` and `text_models_` lookup logic before
  ViewContext grows focused-text mutation helpers.

## 2026-06-30 ViewContext Focused Text Mutation

- A focused-text mutation helper fits the existing capability-forwarding
  `ViewContext` style: it looks up the runtime's focused text model, invokes an
  author callback when present, and returns false for no callback or no focused
  model.
- Tests for the helper should use key-only event sequences because generic text
  input routing tests intentionally insert text between keyboard events.

## 2026-06-30 Public Prelude Header

- `include/cgpui/cgpui.hpp` can stay as a simple aggregate header over the
  existing module headers for now; no new source target or platform-specific
  selection is needed for this authoring convenience.
- Header-cleanliness coverage should instantiate a concrete `View`, which means
  the test fixture must implement both `paint` and `handle_event` from the
  current public `View` contract.
- The prelude intentionally exposes the existing API surface without changing
  ownership or runtime behavior, keeping this slice an additive source
  compatibility improvement.

## 2026-06-30 ViewContext Event Route Helper

- `ViewContext::current_event_route()` should be a snapshot accessor over the
  existing `event_route` field, not a live recomputation from `WindowRuntime`.
- Keeping the public field and adding the helper preserves compatibility while
  giving author code a more intentional GPUI-like route lookup spelling.
- The helper is valid in both view event handlers and after-event callbacks
  because `WindowRuntime::context()` already snapshots `current_event_route_`
  into the context object for both paths.

## 2026-06-30 ViewContext Input State Helper

- `ViewContext::input_state()` should mirror the existing `input` snapshot
  field, preserving the event-time view of focus, pointer, hover, and cursor
  state instead of querying mutable runtime state later.
- Returning the snapshot by value keeps the helper simple and avoids exposing a
  mutable alias into the context object.
- The helper is intentionally additive: existing direct `context.input` access
  remains available while author code gains a clearer accessor spelling.

## 2026-06-30 Steps 129-168 Follow-On Planning

- Steps 129-168 should remain queued behind completion of Steps 99-128 unless
  the roadmap is explicitly reprioritized; Step 129 depends on the model,
  window, route, text, platform, and demo surfaces that Steps 99-128 still need
  to land.
- The post-Step-128 queue is best treated as four 10-step product bands:
  context/entity/global/async ergonomics, keyed reconciliation and reusable
  widgets, text/font/renderer maturity, then Windows/Wayland platform closure
  and parity documentation.
- Step 168 should not claim full GPUI parity. Its exit condition is a practical
  Windows/Linux GPUI-core foundation plus an explicit audit of implemented,
  partial, missing, and Mac/Metal-deferred areas.

## 2026-06-30 Model Entity Aliases

- `Model<T>` and `Entity<T>` should remain pure public authoring aliases over
  `EntityId<T>` at this stage; introducing wrapper objects before Step 100/101
  would create ownership and upgrade semantics before the plan has the model
  helpers and weak handles to support them.
- Header-cleanliness coverage belongs in both `core_header_cleanliness` and the
  public prelude because the aliases are part of the low-level entity API and
  also need to be available to normal GPUI-like authoring code through
  `cgpui/cgpui.hpp`.
- Reusing `EntityId<T>` preserves the existing typed-id separation between
  unrelated model/entity payload types and keeps `EntityStore<T>` monotonic id
  behavior unchanged.

## 2026-06-30 ViewContext Model Helpers

- `new_model`, `read_model`, `update_model`, and `remove_model` fit best as
  header-only `ViewContext` authoring helpers over the existing entity store;
  this keeps Step 100 additive and avoids adding a second model storage path.
- `update_model` should soft-fail when the model id is missing, and should only
  notify model changes after it finds the model and runs the author callback.
  That lets subscribed views receive layout/paint invalidation without making
  failed updates schedule redraws.
- `remove_model` can delegate to existing entity removal semantics, including
  notifying subscribed views only when an actual model is removed.

## 2026-06-30 Back-40 Plan Boundary

- The "后40步" plan is Steps 129-168, not a replacement for the active
  Steps 101-128 queue. Step 129 remains gated on Step 128 being merged and
  verified on both Windows and WSL Arch Linux.
- The post-Step-128 work should keep the current four-band shape:
  context/entity/global/async first, keyed reconciliation/widgets second,
  text/font/renderer maturity third, and Windows/Wayland platform closure plus
  parity audit last.
- The exit condition for Step 168 is a practical Windows/Linux GPUI-core
  foundation with an explicit audit of completed, partial, missing, and
  Mac/Metal-deferred areas; it should not claim full upstream GPUI parity.

## 2026-06-30 Weak Entity And View Handles

- `WeakEntity<T>` should remain a lightweight typed-id wrapper at this stage.
  Upgrade behavior belongs on `WindowRuntime`/`ViewContext`, where the runtime
  can check whether the model/entity store still contains the id.
- `WeakView` can mirror that shape for `ViewId`; the current liveness check is
  `is_view_id_allocated(...)`, which matches the existing monotonic view-id
  allocation model until Step 107 introduces a real multi-view registry.
- Step 101 intentionally does not add observer ownership, subscription tokens,
  or entity handle convenience methods. Those remain Step 102/133/130 work and
  would overbuild this slice.

## 2026-06-30 Back-40 Entry Contract Refresh

- The后 40 步 plan should stay an execution-ready queue for Steps 129-168, not
  an active implementation plan while Step 102-128 still have unresolved
  runtime, model, event, layout, renderer, platform, and demo work.
- The important boundary is the Step 128 handoff: Step 129 should start only
  after the public-prelude demo, model invalidation, view/app lifecycle,
  routing/focus/scroll/layout depth, text command metadata, cursor/clipboard,
  and IME geometry are all merged and verified on Windows and WSL Arch Linux.
- Since Step 102 already implements observation callbacks in its feature
  worktree, the next active implementation step is Step 103: make model
  updates/removals invalidate subscribed views automatically while preserving
  Step 102 observer callback semantics.

## 2026-06-30 Model Observe Helper

- Model observers should be public typed callbacks at the authoring boundary
  (`ModelObserver<T>`) and erased only inside the runtime storage layer, where
  `std::type_index` plus the raw entity id can match notifications without
  duplicating stores per model type.
- Observing a missing model should soft-fail and avoid registering the
  callback. That keeps stale ids from creating future callbacks that look
  alive but cannot read their model.
- Step 102 intentionally keeps automatic subscribed-view invalidation out of
  the observe helper. `notify_entity_changed(...)` still preserves the
  existing layout invalidation behavior, while Step 103 can formalize the
  model-driven subscribed-view invalidation contract without changing the
  observer callback shape.

## 2026-06-30 Model Update Invalidates Subscribed Views

- Once `View::render(ViewContext&)` is the GPUI-like element-tree authoring
  entry point, model/entity notifications for subscribed views need render
  invalidation, not only layout/paint invalidation. Otherwise a model update
  can schedule a frame without rebuilding the rendered element tree.
- `request_render()` is the narrowest existing runtime primitive for this
  contract because it already sets render, layout, and paint invalidation and
  schedules redraw. Reusing it keeps Step 103 small and avoids introducing a
  separate subscribed-view invalidation queue before the view registry lands.
- Observer callbacks remain part of `notify_entity_changed(...)`; Step 103
  changes the invalidation level after a match, while preserving Step 102's
  callback timing and missing-model soft-fail behavior.

## 2026-06-30 Back-40 Planning After Step 103

- The post-Step-128 "back 40" plan is Steps 129-168. It should remain a
  follow-on execution queue, not a replacement for the active Step 104-128
  work.
- Step 129 must stay gated on a clean Step 128 merge plus Windows and WSL Arch
  full debug verification, because the back-40 depends on the AppContext,
  WindowOptions, view registry, routing, layout, text command, cursor,
  clipboard, IME, and public-demo surfaces still scheduled in Steps 104-128.
- The current state for planning is Step 103 complete on `master` at
  `5949c84`, with Step 104 already opened in `.worktrees/app-context-wrapper`
  and baseline targeted tests passing.

## 2026-06-30 AppContext Setup Wrapper

- `AppContext` can start as a deliberately thin public setup wrapper containing
  `WindowRuntime& runtime`; this matches the current context-as-capability
  pattern without duplicating app/window ownership before Step 105.
- Keeping the existing `AppSetupCallback(WindowRuntime&)` field and adding an
  `AppContextSetupCallback` field preserves source compatibility for existing
  runner callers while exposing the GPUI-like setup spelling.
- Running the legacy runtime setup first and the new app-context setup second
  keeps existing behavior stable and gives callers a deterministic order when
  both callbacks are supplied.

## 2026-06-30 Back-40 Planning After Step 104

- The post-Step-128 "后40步" remains Steps 129-168, not a replacement for the
  active Step 105-128 execution queue. Starting Step 129 early would skip
  `WindowOptions`, root-view storage, view registry, child-view placeholders,
  routing ancestry, propagation, focus/scroll/layout depth, renderer text
  metadata, cursor/clipboard/IME surfaces, and the public demo rewrite.
- The next actionable implementation is already set up: continue
  `.worktrees/window-options-open-window` for Step 105 and add RED coverage for
  `WindowOptions` plus `AppContext::open_window(...)`. Recreating the Step 105
  worktree would be churn.
- The back-40 plan is useful as an execution profile now: 129-138 context and
  async, 139-148 keyed elements/widgets, 149-158 text/font/renderer maturity,
  and 159-168 Windows/Wayland closure plus the parity audit. Its entry gate
  stays Step 128 merged and Windows/WSL verified on a clean `master`.

## 2026-06-30 WindowOptions And Open Window Skeleton

- `WindowOptions` can start as a direct wrapper around `WindowDescriptor`.
  That keeps defaults (`CGPUI`, 1280x720) and platform-neutral descriptor
  mapping intact while adding the GPUI-like fluent authoring spelling needed by
  later app/window lifecycle steps.
- Step 105 should record app-opened window descriptors rather than create real
  platform windows. `run_app` still owns one `WindowRuntime`, one platform
  window, and one renderer; Step 106/159 can grow actual root-view and
  multi-window ownership without pretending this skeleton already owns extra
  native windows.
- The setup-time open-window record must remain visible after `run(...)`
  starts because `setup_context` runs before platform window creation. Clearing
  records at the start of `WindowRuntime::run(...)` would erase the only Step
  105 observable state and weaken the handoff to Step 106.

## 2026-06-30 Back-40 Planning After Step 112 GREEN

- The后 40 步 plan remains Steps 129-168 and should still wait behind the Step
  128 gate. Step 112 being GREEN in its feature worktree improves the distance
  estimate but does not open Step 129 yet because full verification, merge, and
  Steps 113-128 are still required.
- The immediate action is to finish Step 112 verification and merge from
  `.worktrees/scroll-element-binding`, then continue Step 113: wheel and
  trackpad scroll routing into bound scroll state.
- After Step 112 merges, the remaining route to the back-40 entry is 16
  implementation steps, Steps 113-128, plus post-Step-128 targeted, Windows
  full debug, and WSL Arch full debug verification on a clean `master`.

## 2026-06-30 Scroll Element Binding Merged

- `ScrollState` is intentionally an alias over `ScrollModel` at Step 112. That
  gives the public authoring API the GPUI-like name without duplicating scroll
  model storage or creating a second offset/update path before event routing
  lands in Step 113.
- `ScrollElement` binds a `ScrollState&` to one child, syncs viewport/content
  sizes during layout, and preserves child hit testing and event forwarding.
  Wheel/trackpad deltas are intentionally still Step 113 so the binding surface
  can be verified before runtime scroll routing mutates it.
- After Step 112 merged and post-merge verification passed, the active 128-step
  route is Step 113 through Step 128. The back-40 entry remains gated behind
  Step 128 and Windows/WSL full verification.

## 2026-06-30 Back-40 Planning After Step 115 Merge

- The后 40 步 plan remains Steps 129-168 and is still gated behind Step 128.
  Step 115 is now fully merged and post-merge verified, so the active
  pre-back-40 route is Steps 116-128 rather than Steps 115-128.
- From `master` at `c443592 feat: add flex alignment justification`, the
  effective distance to Step 129 is 13 implementation steps plus the
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. The distance through Step 168 is 53 implementation steps plus
  the Step 128 exit verification and four 10-step band checkpoint reviews.
- Step 116 should start from `.worktrees/flex-grow-shrink` on
  `codex/flex-grow-shrink` and should first add RED coverage for child flex
  grow/shrink factors affecting main-axis allocation while preserving default
  zero-grow behavior.

## 2026-06-30 Back-40 Planning After Step 121 Docs Closeout

- The后 40 步 plan remains Steps 129-168 and is still gated behind Step 128.
  Step 121 is now behavior-merged at `cf180f4 feat: add text paint command`
  and docs-closed at `dc010b6 docs: mark step 121 merged`, with post-merge
  targeted, Windows full debug, and WSL Arch Linux full debug verification
  already recorded.
- The active pre-back-40 route is exactly Steps 122-128 plus the
  post-Step-128 exit verification. Step 122 should continue the existing clean
  worktree `.worktrees/font-descriptor-font-size` on
  `codex/font-descriptor-font-size`; recreating that worktree would be churn.
- The后 40 步 planning target is four sequential bands: Steps 129-138 for
  context/entity/global/action/subscription/defer/timer/async/batching/
  diagnostics, Steps 139-148 for keyed reconciliation/widgets/style cascade,
  Steps 149-158 for text/font/renderer diagnostics, and Steps 159-168 for
  Win32/Wayland platform completion plus the parity audit.
- The effective distance to Step 129 is 7 implementation slices plus the
  Step 128 exit verification. The effective distance through Step 168 is 47
  implementation slices plus four band checkpoint reviews.

## 2026-07-01 Platform Cursor Application

- `CursorShape` needs to live below the UI layer once platform windows apply
  cursors directly. Moving it from `include/cgpui/ui/ui.hpp` to
  `include/cgpui/core/events.hpp` lets both `WindowRuntime` and
  `PlatformWindow` use the same platform-neutral enum without making platform
  depend on UI.
- The narrow runtime contract is to apply platform cursor changes after routed
  pointer hover recomputes `cursor_shape_`, and only when the shape changes.
  This keeps existing input snapshots intact while avoiding redundant platform
  calls for repeated pointer moves over the same cursor shape.
- Win32 can map the existing cursor shapes to system cursor resources with
  `LoadCursorW`, update the window class cursor through `SetClassLongPtrW`, and
  call `SetCursor` for immediate feedback.
- Wayland should stay a skeleton at Step 124: store the requested shape and
  issue a testable `wl_pointer.set_cursor` request when the pointer is over the
  window, but defer cursor theme loading and cursor surfaces to a later
  platform-depth step.
- The Wayland test compositor must count `set_cursor` requests, not merely
  record a boolean, because pointer enter can apply the default cursor before
  the explicit `PlatformWindow::set_cursor(text)` call.
