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
That plan now includes the post-Step-128 entry contract, branch slugs, first
RED test intent, targeted commands, checkpoint exits, and Windows/WSL
verification matrix for all 40 follow-on steps. It records the current
pre-back-40 handoff state at `08d0fef docs: mark step 109 merged`: Step 109 is
merged and post-merge verified on Windows and WSL Arch Linux, Step 110 is the
next implementation step, and Step 129 remains gated behind completion and
Windows/WSL verification of Steps 110-128.

From the current state, the effective distance to Step 129 is 19 implementation
steps, Steps 110-128, plus post-Step-128 targeted, Windows, and WSL
verification. The effective distance through Step 168 is 59 implementation
steps, Steps 110-168, plus the four follow-on band checkpoint reviews.

These steps are intentionally queued after Step 128. They should not preempt
the current Step 110-128 queue unless the plan is explicitly reprioritized.

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
110. [ ] Event propagation phases: target handling then ancestor bubbling before view fallback.
111. [ ] Focus traversal over enabled focusable elements with Tab and Shift+Tab actions.
112. [ ] Scroll element binding helper backed by `ScrollState`.
113. [ ] Wheel and trackpad scroll routing into bound scroll state.
114. [ ] Hidden overflow participates in hit testing, not only paint clip metadata.
115. [ ] Flex alignment and justification primitives.
116. [ ] Flex grow and shrink factors for child layout.
117. [ ] Absolute positioning and inset style primitive.
118. [ ] Layer/elevation style primitive mapped onto deterministic z order.
119. [ ] Rounded-rect paint command that preserves border radius metadata.
120. [ ] Vulkan renderer honors clip rect metadata for solid rectangles.
121. [ ] Text paint command separates text drawing from placeholder rectangles.
122. [ ] Font descriptor and basic font-size style primitives.
123. [ ] Text element emits caret and selection paint metadata.
124. [ ] Platform cursor application for Win32 and Wayland.
125. [ ] Win32 system clipboard backend for text copy, cut, and paste.
126. [ ] Wayland system clipboard backend skeleton for text copy, cut, and paste.
127. [ ] IME composition/candidate rectangle data from the focused text element.
128. [ ] GPUI-like demo rewrite using the public prelude and new authoring API.
129. [ ] Public `Context<T>` authoring alias over `ViewContext` for view/model code.
130. [ ] Entity handle API with `read`, `update`, and `downgrade` convenience methods.
131. [ ] Global app state registry with typed `set_global`, `global`, and `update_global` helpers.
132. [ ] Scoped action registry for app, window, view, and focused element actions.
133. [ ] Subscription ownership token that disconnects observers on drop/removal.
134. [ ] Deferred callback queue for `cx.defer(...)` style post-event work.
135. [ ] Timer API for one-shot and repeating callbacks through the runtime loop.
136. [ ] Async task handle skeleton with main-thread completion dispatch.
137. [ ] Runtime update batching so multiple model/global changes coalesce redraws.
138. [ ] Public diagnostics snapshot for entities, subscriptions, invalidations, and frames.
139. [ ] Keyed element identity and keyed reconciliation beyond parent-local index matching.
140. [ ] Element lifecycle hooks for mount, update, and unmount notifications.
141. [ ] Element state storage keyed by element id for reusable widgets.
142. [ ] Style class and theme token primitives for reusable design vocabulary.
143. [ ] Style cascade resolution combining base, class, state, and inline styles.
144. [ ] `FocusHandle` primitive with request, release, contains, and focused queries.
145. [ ] Button widget primitive built from public element, focus, style, and action APIs.
146. [ ] Label widget primitive using text style and text paint commands.
147. [ ] Text input widget primitive integrating focus, text model, selection, clipboard, and IME geometry.
148. [ ] Scrollable list container with stable item keys and viewport clipping metadata.
149. [ ] Font database abstraction and platform font discovery skeleton for Win32 and Linux.
150. [ ] Text shaping run abstraction with deterministic fallback metrics before full shaping.
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

Step 109 is merged to `master` and post-merge verified on Windows and WSL Arch
Linux. The active implementation step is Step 110: event propagation phases:
target handling then ancestor bubbling before view fallback.

The post-Step-128 back-40 plan is ready as Steps 129-168 and now has explicit
band checkpoints after Steps 138, 148, 158, and 168. Step 129 remains gated
behind completion and Windows/WSL verification of Steps 110-128.

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
