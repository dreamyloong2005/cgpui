# CGPUI GPUI-Core 128-Step Plan

## Goal

Track small, verified implementation slices toward a Windows/Linux
GPUI-core-like API. Windows and Linux stay on Vulkan, Linux stays on Wayland,
and macOS/Cocoa + Metal remains an explicit readiness boundary until the
Windows/Linux core API is stable enough for parity work.

## Current Baseline

- `master` starts at `779e62c feat: add pointer capture owner`.
- Windows and Linux use Vulkan.
- Linux platform target is Wayland.
- Step 88 adds the explicit macOS/Cocoa + Metal target boundary without making
  macOS parity part of the active Windows/Linux implementation track.

## Definition Of Done For This 20-Step Goal

- All 20 steps below are implemented, committed, merged to `master`, and verified.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI, platform, renderer, or build surfaces.
- `git status --short --branch` on `master` has no tracked/staged changes from the work, aside from the pre-existing untracked `.vscode/`.

## Definition Of Done For The 40-Step Goal

- Steps 1-20 remain implemented and verified on `master`.
- Steps 21-40 below are implemented, committed, merged to `master`, and verified.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI, platform, renderer, or build surfaces.
- `git status --short --branch` on `master` has no tracked/staged changes from the work, aside from the pre-existing untracked `.vscode/`.

## Definition Of Done For The 60-Step Goal

- Steps 1-40 remain implemented and verified on `master`.
- Steps 41-60 below are implemented, committed, merged to `master`, and verified.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI, platform, renderer, or build surfaces.
- `git status --short --branch` on `master` has no tracked/staged changes from the work, aside from the pre-existing untracked `.vscode/`.

## Definition Of Done For The 128-Step Goal

- Steps 1-88 remain implemented and verified on `master`.
- Steps 89-128 below are implemented, committed, merged to `master`, and verified.
- Each implementation step follows the existing RED/GREEN pattern with a
  targeted test that fails for the expected missing API or behavior before the
  implementation lands.
- Windows full debug test suite passes after each merge.
- Linux/WSL full debug test suite passes for slices that touch shared UI,
  platform, renderer, or build surfaces.
- macOS-specific code remains source-isolated behind macOS build guards until
  a dedicated Mac parity run begins.
- `git status --short --branch` on `master` has no tracked/staged changes from
  the work, aside from the pre-existing untracked `.vscode/`.

## Definition Of Done For The 168-Step Follow-On Goal

- Steps 1-128 remain implemented, merged to `master`, and verified on Windows
  and WSL Arch Linux.
- Steps 129-168 below are implemented only after the 128-step goal is complete;
  until then they are a planned follow-on queue, not the active execution focus.
- The follow-on work moves from "close to GPUI core API" toward a practical
  Windows/Linux GPUI-core experience: context/entity ergonomics, keyed
  reconciliation, reusable widgets, text/font rendering depth, async/timer
  integration, diagnostics, and platform-backed Win32/Wayland behavior.
- Windows and Linux continue to use Vulkan; Linux continues to target Wayland.
- macOS/Cocoa + Metal remains a readiness boundary and must not become a parity
  requirement inside this follow-on Windows/Linux track.
- Each implementation step follows the same RED/GREEN, feature-worktree,
  merge, Windows verification, and WSL verification discipline used for Steps
  89-128.

## Steps 89-128 Execution Roadmap

Detailed execution plan:
`docs/superpowers/plans/2026-06-30-gpui-core-steps-89-128-execution-plan.md`.

- Band A, Steps 89-98: authoring ergonomics and render entry. This lands the
  GPUI-like public surface for elements, style helpers, event handler
  shortcuts, style states, `View::render(ViewContext&)`, rendered tree
  installation, and render invalidation observability.
- Band B, Steps 99-108: model, app, window, and view lifecycle. This introduces
  public model aliases, context model helpers, weak handles, observations,
  model-driven invalidation, `AppContext`, `WindowOptions`, root-view
  lifecycle storage, a view registry, and child-view placeholders.
- Band C, Steps 109-118: event propagation, focus traversal, scroll routing,
  hidden-overflow hit testing, and deeper layout controls. This makes route
  ancestry, bubbling, Tab traversal, scroll state binding, flex alignment,
  grow/shrink, absolute positioning, and layer/elevation behavior explicit.
- Band D, Steps 119-128: render command depth, text metadata, platform cursor
  and clipboard backends, IME geometry, and the public-prelude demo rewrite.
  This is the Windows/Linux backend-hardening band while macOS remains a
  readiness boundary.

## Steps 129-168 Follow-On Roadmap

Detailed follow-on plan:
`docs/superpowers/plans/2026-06-30-gpui-core-steps-129-168-forward-plan.md`.
That plan now includes branch slugs, first RED test intent, targeted commands,
checkpoint exits, and Windows/WSL verification matrix for all 40 follow-on
steps. The pre-back-40 handoff is complete: Steps 89-128 are merged to
`master` and post-merge verified on Windows and WSL Arch Linux.

Step 139, keyed element identity and keyed reconciliation beyond parent-local
index matching, is merged on `master` at
`8695bb1 feat: add keyed element identity` and post-merge verified on Windows
and WSL Arch Linux. RED failed as expected on missing `ElementKey`,
`Element::key()`, `ElementBuilder::key(...)`, and
`ElementTree::reconcile_children(...)`. GREEN adds optional stable element
keys, builder key propagation through event/focus wrappers, and parent-local
batch reconciliation that preserves keyed child ids across
reorder/insert/remove while keeping index-based reconciliation available for
existing unkeyed call sites. The effective distance through Step 168 is 29
remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 140, element lifecycle hooks for mount, update, and unmount
notifications, is merged on `master` at
`a179f5a feat: add element lifecycle hooks` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`ElementLifecycleContext` and lifecycle hook APIs. GREEN adds no-op
`Element::on_mount(...)`, `on_update(...)`, and `on_unmount(...)` hooks plus
mount/update/unmount dispatch from root, index child, keyed child, subtree
removal, and `set_root(...)` replacement paths. The effective distance through
Step 168 is 28 remaining follow-on implementation steps plus the four
follow-on band checkpoint reviews.

Step 141, element state storage keyed by element id for reusable widgets, is
merged on `master` at `10415c6 feat: add element state storage` and
post-merge verified on Windows and WSL Arch Linux. RED failed as expected on
missing `ElementTree::state<T>(...)`, `state_or_init<T>(...)`, and
`emplace_state<T>(...)` APIs. GREEN adds per-node type-indexed state storage,
soft-fail lookup/init/replace helpers, and `WindowRuntime` /
`WindowRuntimeContext` forwarding for runtime-owned element trees. The
effective distance through Step 168 is 27 remaining follow-on implementation
steps plus the four follow-on band checkpoint reviews.

Step 142, style class and theme token primitives for reusable design
vocabulary, is merged on `master` at
`1493c91 feat: add style classes theme tokens` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing `StyleClassId`,
`style_class(...)`, `StyleClasses`, `ThemeTokenId`, `theme_token(...)`, and
`Theme` color/spacing token APIs. GREEN adds inert public style vocabulary
primitives: named class ids, ordered de-duplicated class lists, named theme
token ids, and typed color/spacing token storage with missing-token soft
failure. The effective distance through Step 168 is 26 remaining follow-on
implementation steps plus the four follow-on band checkpoint reviews.

Step 143, style cascade resolution combining base, class, state, and inline
styles, is merged on `master` at
`2ab43a7 feat: add style cascade resolution` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing `StyleCascade`,
class/inline builder APIs, and styled-element resolved-style APIs. GREEN adds
deterministic `StyleCascade` resolution plus `StyledElement` storage for
classes and inline overlays, while leaving runtime layout/paint cascade
installation for a later slice. The effective distance through Step 168 is 25
remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 144, `FocusHandle` primitive with request, release, contains, and focused
queries, is merged on `master` at
`874ef1f feat: add focus handle primitive` and post-merge verified on Windows
and WSL Arch Linux. RED failed as expected on missing `FocusHandle` and
`focus_handle(...)` APIs. GREEN adds a lightweight element-id focus handle,
runtime/context `focus_handle(...)` factories, focus request/release
forwarding, and query helpers over `ViewInputState` plus a public runtime
input snapshot. The effective distance through Step 168 is 24 remaining
follow-on implementation steps plus the four follow-on band checkpoint reviews.

Step 145, button widget primitive built from public element, focus, style, and
action APIs, is merged on `master` at
`da62f61 feat: add button widget primitive` and post-merge verified on Windows
and WSL Arch Linux. RED failed as expected on missing `cgpui::button`,
`ButtonElement`, and button/runtime action dispatch behavior. GREEN adds
`ButtonElement`, fluent `ButtonBuilder`, public `button(...)`, style-state
metadata, disabled/focusable/click/action behavior, and button style-box paint
metadata shared with styled elements. The effective distance through Step 168
is 23 remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 146, label widget primitive using text style and text paint commands, is
merged on `master` at `ca63320 feat: add label widget primitive` and
post-merge verified on Windows and WSL Arch Linux. RED failed as expected on
missing `cgpui::label` and `LabelElement` APIs. GREEN adds an owned-text
`LabelElement`, fluent `LabelBuilder`, public `label(...)`, text style
shortcuts, key/disabled builder support, and paint behavior that emits a text
command without caret or selection metadata. The effective distance through
Step 168 is 22 remaining follow-on implementation steps plus the four
follow-on band checkpoint reviews.

Step 147, text input widget primitive integrating focus, text model,
selection, clipboard, and IME geometry, is merged on `master` at
`e3f122b feat: add text input widget primitive` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`cgpui::text_input` and `TextInputElement` APIs. GREEN adds a focusable
`TextInputElement`, fluent `TextInputBuilder`, public `text_input(...)`,
text style/key/disabled builder support, focused text-model fallback through
installed text input elements, and runtime routing for text input, edit
bindings, clipboard operations, and IME geometry without requiring manual
`bind_text_model(...)`. The effective distance through Step 168 is 21
remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 148, scrollable list container with stable item keys and viewport
clipping metadata, is merged on `master` at
`447b74c feat: add scrollable list container` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing
`cgpui::scrollable_list` and `ScrollableListElement` APIs. GREEN adds
`ScrollableListElement`, fluent `ScrollableListBuilder`, public
`scrollable_list(ScrollState&)`, keyed item installation, scroll-state
viewport/content updates, offset-adjusted child bounds, and viewport clip
metadata around item paint commands. The effective distance through Step 168
is 20 remaining follow-on implementation steps plus the four follow-on band
checkpoint reviews.

Step 149, font database abstraction and platform font discovery skeleton for
Win32 and Linux, is merged on `master` at
`b415784 feat: add font database skeleton` and post-merge verified on Windows
and WSL Arch Linux. RED failed as expected on missing `FontDatabase`,
`FontFaceDescriptor`, `FontSource`, `discover_test_fonts(...)`, and
`PlatformApplication::discover_fonts()` APIs. GREEN adds a platform-neutral
font database and deterministic test discovery helper in `ui/text.hpp`, plus
gracefully empty Win32 and Wayland discovery overrides. The effective distance
through Step 168 is 19 remaining follow-on implementation steps plus the four
follow-on band checkpoint reviews.

Step 150, text shaping run abstraction with deterministic fallback metrics
before full shaping, is merged on `master` at
`d5a3c57 feat: add text shaping run abstraction` and post-merge verified on
Windows and WSL Arch Linux. RED failed as expected on missing `TextShapeRun`,
`shape_text(...)`, and text element shaping APIs. GREEN adds `TextGlyphRun`,
`TextShapeRun`, UTF-8 codepoint-style fallback glyph grouping,
font-size-derived fallback advances, and `TextElement`/`LabelElement` layout
through shaping runs. The effective distance through Step 168 is 18 remaining
follow-on implementation steps plus the four follow-on band checkpoint
reviews.

- Band E, Steps 129-138: GPUI-like context, entity, global state, action
  scoping, subscriptions, and async/timer primitives.
- Band F, Steps 139-148: keyed reconciliation, element lifecycle hooks,
  style classes/themes, focus handles, and reusable widget primitives.
- Band G, Steps 149-158: text/font rendering maturity, glyph cache surfaces,
  renderer command batching, opacity/transform, and frame diagnostics.
- Band H, Steps 159-168: Windows/Wayland platform completion, multi-window
  lifecycle, accessibility/debug surfaces, packaging checks, and API parity
  documentation.

## Steps

1. [x] EventResult: make view event handlers return consumed/cancelled state.
2. [x] Event dispatch observability: expose last event result to runtime tests and callbacks where useful.
3. [x] Event routing shell: add a root-level dispatch function that can later route to element/view ids.
4. [x] EntityId and EntityStore skeleton: typed ids, insert/get/remove lifecycle.
5. [x] Context entity access: expose safe entity mutation/read helpers through a context object.
6. [x] View identity allocation: allocate stable ids beyond the root view.
7. [x] ElementId and Element base API: minimal polymorphic element contract.
8. [x] Element tree container: store children, parent links, and root element id.
9. [x] Element reconcile pass: preserve ids across simple rebuilds.
10. [x] LayoutInput/LayoutOutput primitives: constraints, size, origin.
11. [x] Layout pass for fixed-size leaf elements.
12. [x] Layout pass for simple vertical stack.
13. [x] Hit testing over laid-out element bounds.
14. [x] Pointer routing to hit-tested element id.
15. [x] Pointer capture routing to owner id.
16. [x] Keyboard routing to focus owner id.
17. [x] Action/Command primitive: named actions and dispatch result.
18. [x] Key binding table: map key events to actions.
19. [x] Basic text model: editable UTF-8 buffer with cursor.
20. [x] Text input routing into focused text model.
21. [x] Style primitives: colors, size, padding, and border fields.
22. [x] Element builder API skeleton for composing styled elements.
23. [x] Styled element paints a solid background rect.
24. [x] Padding participates in styled element layout.
25. [x] Border width/color and border radius primitives.
26. [x] Paint tree generation from element hierarchy.
27. [x] Clip rect and overflow primitives.
28. [x] Z-order and deterministic child paint order.
29. [x] Hover state tracking for routed pointer targets.
30. [x] Cursor shape API and routed cursor state.
31. [x] Scroll state/model primitive.
32. [x] Flex row/column layout basics.
33. [x] Runtime invalidation requests for layout and paint.
34. [x] Update scheduling that requests redraw after model/view changes.
35. [x] View/model subscription relation skeleton.
36. [x] Text selection range model.
37. [x] Text key editing actions for delete, movement, and selection.
38. [x] Clipboard abstraction for Win32 and Wayland.
39. [x] IME composition skeleton for Win32 and Wayland.
40. [x] Real interactive demo using the new API surface.
41. [x] ElementTree root layout helper.
42. [x] ElementTree root hit-test helper.
43. [x] Element tree owned runtime root installation.
44. [x] Runtime lays out installed element trees on redraw.
45. [x] Element event handler hook.
46. [x] Runtime dispatches routed element events before view fallback.
47. [x] Element builder flex row/column helpers.
48. [x] Element builder vertical stack helper.
49. [x] Element builder fixed-size helper.
50. [x] Gap style primitive for stack and flex layout.
51. [x] Margin style primitive and outer layout sizing.
52. [x] Border paint emission from styled elements.
53. [x] Hidden overflow clip metadata in paint commands.
54. [x] Text element skeleton bound to TextModel.
55. [x] Runtime text edit action routing for focused text models.
56. [x] Runtime clipboard paste into focused text models.
57. [x] Runtime clipboard copy from focused text selections.
58. [x] Runtime clipboard cut from focused text selections.
59. [x] ViewContext convenience wrapper for common runtime APIs.
60. [x] Public app runner skeleton for GPUI-like startup.
61. [x] Text element builder helper.
62. [x] Focusable element activation hook.
63. [x] Runtime clicks request focus for focusable elements.
64. [x] Element enabled/disabled state primitive.
65. [x] Disabled elements skip event handling.
66. [x] Element tree preorder traversal helper.
67. [x] Element tree find-by-id convenience helper.
68. [x] ViewContext text model binding helper.
69. [x] ViewContext element tree installation helper.
70. [x] ViewContext action registration helper.
71. [x] ViewContext key binding helper.
72. [x] ViewContext text edit binding helper.
73. [x] ViewContext cursor binding helper.
74. [x] ViewContext focus request/release element helpers.
75. [x] ViewContext pointer capture element helpers.
76. [x] Element builder click handler helper.
77. [x] Element builder focusable helper.
78. [x] Element builder key handler helper.
79. [x] Element builder disabled convenience helper.
80. [x] Element tree enabled descendant traversal query.
81. [x] Runtime skips disabled elements for focus activation.
82. [x] Runtime clears hover cursor when hovered element becomes disabled.
83. [x] Runtime text model lookup helper for focused element.
84. [x] ViewContext focused text model mutation helper.
85. [x] Public prelude header for core authoring APIs.
86. [x] ViewContext event route access helper.
87. [x] ViewContext input state access helper.
88. [x] Mac readiness audit and desktop target mapping.
89. [x] Public `AnyElement` alias and minimal into-element authoring convention.
90. [x] Free authoring factories for `div()`, `h_flex()`, `v_flex()`, `v_stack()`, and `text(...)`.
91. [x] Element builder child overloads for builders, `AnyElement`, and element ownership.
92. [x] Style unit and color helpers: `px`, `rgb`, `rgba`, and edge constructors.
93. [x] Element builder fluent style shortcuts for size, spacing, background, border, radius, and gap.
94. [x] Pointer handler shortcuts for down, up, move, and click authoring paths.
95. [x] Focus, hover, and disabled style-state overlay primitives.
96. [x] GPUI-like `View::render(ViewContext&)` hook skeleton while preserving the current view contract.
97. [x] Runtime render pass installs the root view's rendered element tree.
98. [x] ViewContext render invalidation helper and after-render observability.
99. [x] Public `Model<T>`/`Entity<T>` authoring aliases over typed entity ids.
100. [x] ViewContext model helpers for create, read, update, and remove.
101. [x] Weak entity/view handle primitives with soft-fail upgrade.
102. [x] ViewContext observe/subscribe callback helper for model changes.
103. [x] Model update notification automatically invalidates subscribed views.
104. [x] Public `AppContext` wrapper over the app runner setup phase.
105. [x] `WindowOptions` builder and `AppContext::open_window(...)` helper skeleton.
106. [x] Runtime root view lifecycle storage for app-opened windows.
107. [x] View registry skeleton for multiple view ids beyond the root.
108. [x] Child-view element placeholder that embeds another view's rendered output.
109. [x] Event route carries element and view ancestry metadata.
110. [x] Event propagation phases: target handling then ancestor bubbling before view fallback.
111. [x] Focus traversal over enabled focusable elements with Tab and Shift+Tab actions.
112. [x] Scroll element binding helper backed by `ScrollState`.
113. [x] Wheel and trackpad scroll routing into bound scroll state.
114. [x] Hidden overflow participates in hit testing, not only paint clip metadata.
115. [x] Flex alignment and justification primitives.
116. [x] Flex grow and shrink factors for child layout.
117. [x] Absolute positioning and inset style primitive.
118. [x] Layer/elevation style primitive mapped onto deterministic z order.
119. [x] Rounded-rect paint command that preserves border radius metadata.
120. [x] Vulkan renderer honors clip rect metadata for solid rectangles.
121. [x] Text paint command separates text drawing from placeholder rectangles.
122. [x] Font descriptor and basic font-size style primitives.
123. [x] Text element emits caret and selection paint metadata.
124. [x] Platform cursor application for Win32 and Wayland.
125. [x] Win32 system clipboard backend for text copy, cut, and paste.
126. [x] Wayland system clipboard backend skeleton for text copy, cut, and paste.
127. [x] IME composition/candidate rectangle data from the focused text element.
128. [x] GPUI-like demo rewrite using the public prelude and new authoring API.
129. [x] Public `Context<T>` authoring alias over `ViewContext` for view/model code.
130. [x] Entity handle API with `read`, `update`, and `downgrade` convenience methods.
131. [x] Global app state registry with typed `set_global`, `global`, and `update_global` helpers.
132. [x] Scoped action registry for app, window, view, and focused element actions.
133. [x] Subscription ownership token that disconnects observers on drop/removal.
134. [x] Deferred callback queue for `cx.defer(...)` style post-event work.
135. [x] Timer API for one-shot and repeating callbacks through the runtime loop.
136. [x] Async task handle skeleton with main-thread completion dispatch.
137. [x] Runtime update batching so multiple model/global changes coalesce redraws.
138. [x] Public diagnostics snapshot for entities, subscriptions, invalidations, and frames.
139. [x] Keyed element identity and keyed reconciliation beyond parent-local index matching.
140. [x] Element lifecycle hooks for mount, update, and unmount notifications.
141. [x] Element state storage keyed by element id for reusable widgets.
142. [x] Style class and theme token primitives for reusable design vocabulary.
143. [x] Style cascade resolution combining base, class, state, and inline styles.
144. [x] `FocusHandle` primitive with request, release, contains, and focused queries.
145. [x] Button widget primitive built from public element, focus, style, and action APIs.
146. [x] Label widget primitive using text style and text paint commands.
147. [x] Text input widget primitive integrating focus, text model, selection, clipboard, and IME geometry.
148. [x] Scrollable list container with stable item keys and viewport clipping metadata.
149. [x] Font database abstraction and platform font discovery skeleton for Win32 and Linux.
150. [x] Text shaping run abstraction with deterministic fallback metrics before full shaping.
151. [ ] Glyph atlas/cache interface shared by text elements and Vulkan renderer.
152. [ ] Vulkan text draw path consumes text paint commands through cached glyph metadata.
153. [ ] Opacity and transform paint metadata with deterministic command ordering.
154. [ ] Renderer command batching by clip, opacity, transform, and primitive kind.
155. [ ] Frame timing and paint/layout/render statistics exposed through diagnostics.
156. [ ] HiDPI scale propagation into layout, text metrics, and renderer resources.
157. [ ] Snapshot tests for paint command streams emitted by the demo and widgets.
158. [ ] Renderer fallback path for unsupported commands with explicit diagnostics.
159. [ ] Multi-window runtime registry with per-window root view and renderer ownership.
160. [ ] Window activation, focus, minimize, restore, and close lifecycle events.
161. [ ] Win32 IME composition window placement wired to focused text geometry.
162. [ ] Wayland text-input/IME protocol skeleton wired to focused text geometry.
163. [ ] Win32 drag-and-drop text/file event skeleton.
164. [ ] Wayland data-device drag-and-drop text/file event skeleton.
165. [ ] Platform event loop wakeup API for timers, async completions, and deferred callbacks.
166. [ ] Accessibility tree skeleton for labels, buttons, text inputs, and focus state.
167. [ ] Windows/Linux demo smoke tests covering window, input, text, clipboard, and redraw flows.
168. [ ] GPUI-core API parity audit document with remaining gaps and Mac parity handoff boundaries.

## Active Step

Current handoff: Step 150, text shaping run abstraction with deterministic
fallback metrics before full shaping, is merged on `master` at
`d5a3c57 feat: add text shaping run abstraction` and post-merge verified on
Windows and WSL Arch Linux. Post-merge targeted tests passed 4/4, Windows
full debug passed 29/29, and WSL Arch Linux full debug passed 26/26. Step 151,
glyph atlas/cache interface shared by text elements and Vulkan renderer, is
the next implementation slice after docs closeout and cleanup.

Step 115, flex alignment and justification primitives, is merged on `master`
at `c443592 feat: add flex alignment justification`. RED failed as expected
on missing `AlignItems`/`JustifyContent` API, feature-worktree targeted tests
passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux full debug
passed 26/26. Post-merge targeted tests passed 3/3, Windows full debug passed
29/29, and WSL Arch Linux full debug passed 26/26. The Step 115 worktree and
feature branch have been removed.

Step 116, flex grow and shrink factors for child layout, is merged on `master`
at `2806a4a feat: add flex grow shrink layout`. RED failed as expected on
missing `flex_grow`/`flex_shrink` style, overlay, builder, and element APIs.
Feature-worktree targeted tests passed 3/3, Windows full debug passed 29/29,
and WSL Arch Linux full debug passed 26/26. Post-merge targeted tests passed
3/3, Windows full debug passed 29/29, and WSL Arch Linux full debug passed
26/26. The Step 116 worktree and feature branch have been removed.

Step 117, absolute positioning and inset style primitive, is merged on
`master` at `210c85d feat: add absolute positioning insets`. RED failed as
expected on missing `Position`, `Style::position`, `Style::inset`, overlay,
builder, and element APIs. Feature-worktree targeted tests passed 3/3, Windows
full debug passed 29/29, and WSL Arch Linux full debug passed 26/26.
Post-merge targeted tests passed 3/3, Windows full debug passed 29/29, and WSL
Arch Linux full debug passed 26/26. The Step 117 worktree and feature branch
have been removed.

The post-Step-128 back-40 plan is ready as Steps 129-168 and now has explicit
completion targets, non-goals, branch/test/verification matrix, per-step exit
artifacts, explicit keep-out-of-scope notes, Step 129 start packet, band
checkpoints after Steps 138, 148, 158, and 168, and an execution gate that
records Steps 115-123 as merged and post-merge verified. Step 129 remains
gated behind completion plus Windows/WSL verification of Steps 124-128 and the
post-Step-128 exit verification.

Step 118, layer/elevation style primitive mapped onto deterministic z order,
is merged on `master` at `9dfc2e7 feat: add layer elevation z order`. RED
failed as expected on missing `Style::layer`, `StyleOverlay::layer`,
`with_layer(...)`, builder `.layer(...)`, and `Element::layer()`/`z_order()`
APIs. Feature-worktree targeted tests passed 3/3, Windows full debug passed
29/29, and WSL Arch Linux full debug passed 26/26. Post-merge targeted tests
passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux full debug
passed 26/26.

Step 119, rounded-rect paint command that preserves border radius metadata, is
merged on `master` at `0893600 feat: add rounded rect paint command`. RED
failed as expected on missing `PaintCommandKind`, `RoundedRect`,
`PaintCommand::kind`, `PaintCommand::rounded_rect`, and
`PaintList::fill_rounded_rect(...)`. Feature-worktree targeted tests passed
3/3, Windows full debug passed 29/29, and WSL Arch Linux full debug passed
26/26. Post-merge targeted tests passed 3/3, Windows full debug passed 29/29,
and WSL Arch Linux full debug passed 26/26.

Step 120, Vulkan renderer honors clip rect metadata for solid rectangles, is
merged on `master` at `9aba0e6 feat: honor vulkan solid rect clips`. RED
failed as expected on missing `SolidRect::clip_rect`. Feature-worktree targeted
tests passed 4/4, Windows full debug passed 29/29, and WSL Arch Linux full
debug passed 26/26. Post-merge targeted tests passed 4/4, Windows full debug
passed 29/29, and WSL Arch Linux full debug passed 26/26.

Step 121, text paint command separates text drawing from placeholder
rectangles, is merged on `master` at `cf180f4 feat: add text paint command`.
RED failed as expected on missing text paint command APIs before the
implementation. GREEN adds `PaintCommandKind::text`, `TextPaint`,
`PaintList::fill_text(...)`, text clip metadata, `TextElement` text-command
painting, and a render path that skips text commands until a later Vulkan text
drawing step. Feature-worktree targeted tests passed 3/3, Windows full debug
passed 29/29, and WSL Arch Linux full debug passed 26/26. Post-merge targeted
tests passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux full
debug passed 26/26. Step 122, font descriptor and basic font-size style
primitives, was the next implementation slice.

Step 122, font descriptor and basic font-size style primitives, is merged on
`master` at `58561b1 feat: add font size style`. RED failed as expected on
missing
`FontDescriptor`, `Style::font`, `Style::font_size`, `StyleOverlay` font
overrides, builder `.font(...)`/`.font_size(...)`, text font metrics, and text
paint font metadata. GREEN adds public font metadata to `Style`/`StyleOverlay`,
deterministic font-size-derived `TextElement` metrics, text builder font
shortcuts, and `TextPaint` font/font-size metadata while preserving the default
16px text metrics. Feature-worktree targeted tests passed 3/3, Windows full
debug passed 29/29, and WSL Arch Linux full debug passed 26/26. Post-merge
targeted tests passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux
full debug passed 26/26. Step 123, text element caret and selection paint
metadata, is the next implementation slice.

Step 123, text element caret and selection paint metadata, is merged on
`master` at `b0b9e00 feat: add text caret selection paint`. RED failed as
expected on missing
`PaintCommandKind::text_selection`, `PaintCommandKind::text_caret`,
`TextSelectionPaint`, `TextCaretPaint`, `PaintCommand` payloads, and
`PaintList::fill_text_selection(...)`/`fill_text_caret(...)`. GREEN adds
metadata-only text selection and caret paint commands, deterministic
font-size-derived byte-offset geometry, caret emission for empty bound text
models, and renderer skipping for text-class commands. Feature-worktree
targeted tests passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux
full debug passed 26/26. Post-merge targeted tests passed 3/3, Windows full
debug passed 29/29, and WSL Arch Linux full debug passed 26/26. Step 124,
platform cursor application for Win32 and Wayland, is the next implementation
slice.

Step 124, platform cursor application for Win32 and Wayland, is merged on
`master` at `74ad787 feat: apply platform cursors`. RED failed as expected on
missing `PlatformWindow::set_cursor(...)`; GREEN moves `CursorShape` into the
core event surface, adds the platform cursor API, applies runtime hover cursor
changes to the platform window, maps Win32 cursor shapes to system cursors, and
adds a Wayland `wl_pointer.set_cursor` skeleton hook without cursor theme
loading. Feature-worktree targeted tests passed on Windows and WSL Arch Linux,
Windows full debug passed 29/29, and WSL Arch Linux full debug passed 26/26.
Post-merge targeted tests passed on Windows for built targets 2/2, Windows
full debug passed 29/29, and WSL Arch Linux full debug passed 26/26. Step 125,
Win32 system clipboard backend for text copy, cut, and paste, is the next
implementation slice.

Step 125, Win32 system clipboard backend for text copy, cut, and paste, is
merged on `master` at `389b9fb feat: add win32 system clipboard`. RED failed as
expected once the test target was force-rebuilt: `clipboard_test/default`
distinguished the existing memory clipboard from Win32 system clipboard
interop. GREEN keeps `MemoryClipboard` unchanged, makes Windows
`create_platform_clipboard()` return a `CF_UNICODETEXT`-backed UTF-8 clipboard
implementation, and updates xmake Windows `user32` links for clipboard
consumers. Feature-worktree targeted tests passed 1/1, Windows full debug
passed 29/29, and WSL Arch Linux full debug passed 26/26. Post-merge targeted
tests passed 1/1, Windows full debug passed 29/29, and WSL Arch Linux full
debug passed 26/26. Step 126, Wayland system clipboard backend skeleton for
text copy, cut, and paste, is the next implementation slice.

Step 126, Wayland system clipboard backend skeleton for text copy, cut, and
paste, is merged on `master` at `ab464d5 feat: add wayland clipboard
skeleton`. RED failed as expected on missing Linux `WaylandClipboard`,
`WaylandClipboardSupport`, and `WaylandClipboardOptions` APIs. GREEN adds a
Linux-only Wayland clipboard skeleton that reports `unsupported`, `no_seat`, or
`available` support states, keeps graceful memory fallback read/write behavior
for unsupported/no-seat runtime copy/cut/paste, and makes Linux
`create_platform_clipboard()` return the Wayland skeleton instead of
`MemoryClipboard`. Feature-worktree targeted tests passed 1/1 on Windows and
WSL Arch Linux, Windows full debug passed 29/29, and WSL Arch Linux full debug
passed 26/26. Post-merge targeted tests passed 1/1, Windows full debug passed
29/29, and WSL Arch Linux full debug passed 26/26.

Step 127, IME composition/candidate rectangle data from the focused text
element, is merged on `master` at `80aadae feat: add focused text ime rect`.
RED failed as expected on missing `ImeCandidateRect` and
`focused_text_ime_rect()` APIs. GREEN adds public `ImeCandidateRect` metadata
plus `WindowRuntime::focused_text_ime_rect()` and
`WindowRuntimeContext::focused_text_ime_rect()`, deriving the candidate rect
from the focused `TextElement` layout bounds, cursor byte offset,
font-size-derived glyph width, and caret height. Feature-worktree targeted
tests passed 3/3, Windows full debug passed 29/29, and WSL Arch Linux full
debug passed 26/26. Post-merge targeted tests passed 3/3, Windows full debug
passed 29/29, and WSL Arch Linux full debug passed 26/26. Step 128,
GPUI-like demo rewrite using the public prelude and new authoring API, is the
next implementation slice.

Step 128, GPUI-like demo rewrite using the public prelude and new authoring
API, is merged on `master` at
`4026899 feat: rewrite demo with public prelude`. RED failed as expected after
the architecture test began requiring `cgpui/cgpui.hpp`, `run_app`,
`AppRunnerOptions`, `AppContext`, `View::render(ViewContext&)`, public element
factories, fluent builder shortcuts, and `ViewContext` model/text helpers
while rejecting manual `WindowRuntime`/`ElementTree` demo setup. GREEN rewrites
`examples/hello_window` around the public prelude and `run_app`, preserves the
first-frame, resize, close, and injected-text smoke env vars, and updates
README foundation wording. Feature-worktree targeted tests passed 2/2, Windows
hello-window smoke tests passed 3/3, Windows full debug passed 29/29, and WSL
Arch Linux full debug passed 26/26. Post-merge targeted tests passed 2/2,
Windows full debug passed 29/29, and WSL Arch Linux full debug passed 26/26.
Step 129, public `Context<T>` authoring alias over `ViewContext`, is the next
implementation slice.

## Risks

- Keep old APIs source-compatible where practical while adding result-returning semantics.
- Avoid building a full view tree before the routing primitives exist.
- Keep each step small enough for Windows and WSL verification.
- Keep macOS readiness as a boundary/audit track until the Windows/Linux core
  API surface is stable enough to justify Metal parity work.
- Do not overfit authoring helpers to the current demo; each helper should be
  general enough to survive later view/model lifecycle work.
- Keep renderer work command-driven and platform-neutral first, then teach the
  Vulkan backend to consume the new commands.
