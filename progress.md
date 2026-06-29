# CGPUI GPUI-Core Progress

## 2026-06-29

- Created the 20-step plan for the Windows/Linux GPUI-core milestone.
- Started Step 1: EventResult.
- Completed Step 1 implementation in `codex/core-event-result`.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 19/19.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 16/16.
- Started Step 2: Event dispatch observability.
- Added design and implementation plan docs for event dispatch records and the after-event callback.
- Added a RED runtime test for `EventDispatchRecord`, `EventKind`, `WindowRuntimeContext::last_event_dispatch`, and `WindowRuntime::set_after_event_callback`; the test failed because those APIs did not exist.
- Implemented Step 2 in `codex/event-dispatch-observability`: dispatch records include sequence, target root view id, event kind, and event result; runtime after-event callbacks see the current record after view handling.
- Fixed a test-only aggregation bug where a boolean accumulator started as `false`, then re-ran the target test.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 19/19.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 16/16.
- Started Step 3: Event routing shell.
- Added design and implementation plan docs for the root event routing shell.
- Added a RED runtime test for `EventRoute`, `EventRouter::route_to_root`, `WindowRuntimeContext::event_route`, and `EventDispatchRecord::route`; the test failed because those APIs did not exist.
- Implemented Step 3 in `codex/event-routing-shell`: view-dispatched events route through a root-only `EventRouter`, runtime contexts expose the active route, and dispatch records store the route used for the dispatch.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default render_view_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 19/19.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 16/16.
- Started Step 4: EntityId and EntityStore skeleton.
- Added design and implementation plan docs for typed entity ids and the minimal entity store lifecycle.
- Added a RED core test for `EntityId<T>`, `EntityStore<T>::insert`, mutable/const `get`, `remove`, and monotonic id allocation; the test failed because `cgpui/core/entity.hpp` did not exist.
- Implemented Step 4 in `codex/entity-store-skeleton`: `EntityId<T>` is a typed id wrapper, and `EntityStore<T>` supports header-only insert/emplace/get/remove with non-reused monotonic ids.
- Added the `entity_store_test` xmake target and included the new header in `core_header_cleanliness`.
- Verified targeted tests: `xmake test -P . entity_store_test/default core_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 20/20.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 17/17.

## 2026-06-30

- Started the second GPUI-core milestone: extend the completed 20-step
  Windows/Linux base to Step 40.
- Updated `task_plan.md` with Steps 21-40 and set the active step to Step 21:
  Style primitives.
- Started Step 21: Style primitives.
- Added a RED `style_test` and `style_test` xmake target for `EdgeSizes`,
  inert `Style` fields, fluent style setters, and UI header cleanliness; the
  test failed because `cgpui/ui/style.hpp` did not exist.
- Implemented Step 21 in `codex/style-primitives`: added header-only
  `EdgeSizes` and `Style` primitives with optional background/foreground color,
  preferred size, padding, and border width.
- Verified targeted tests: `xmake test -P . style_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 24/24.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 21/21.
- Started Step 28: Z-order and deterministic child paint order.
- Added RED `style_test` and `element_test` coverage for default integer
  `Style::z_index`, fluent `with_z_index(int)`, and stable sibling paint order
  sorted by z-index while preserving insertion order for equal z-index values;
  the test failed because `z_index` APIs did not exist.
- Implemented Step 28 in `codex/z-order-paint-order`: added inert z-index
  style data, exposed `Element::z_index()`, made `StyledElement` return its
  style z-index, and made `ElementTree::paint` stable-sort same-parent
  children before recursive painting.
- Verified targeted tests: `xmake test -P . style_test/default element_test/default ui_header_cleanliness/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 24/24.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 21/21.
- Started Step 29: Hover state tracking for routed pointer targets.
- Added RED `window_runtime_test` coverage for `ViewInputState::hovered_element_id`,
  pointer-move hit tracking, clearing hover when the pointer leaves the element
  root, and keeping hover tied to the live hit-test target while pointer
  capture routes events to the captured element; the test failed because the
  hover input field did not exist.
- Implemented Step 29 in `codex/hover-state-tracking`: runtime stores the
  currently hovered `ElementId`, updates it from pointer-move hit testing, clears
  it when no element is hit, exposes it through `WindowRuntimeContext::input`,
  and keeps pointer-capture route overrides separate from hover state.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 24/24.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 21/21.
- Started Step 30: Cursor shape API and routed cursor state.
- Added RED `window_runtime_test` coverage for `CursorShape`,
  `ViewInputState::cursor_shape`, `WindowRuntime::set_element_cursor`, and
  hover-driven cursor changes that restore the default arrow when no element is
  hovered; the test failed because those APIs did not exist.
- Implemented Step 30 in `codex/cursor-shape-state`: added the runtime cursor
  shape enum, exposed current cursor shape through input state, stored
  element-to-cursor bindings by `ElementId`, and updated cursor state alongside
  pointer-move hover hit testing.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 24/24.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 21/21.
- Started Step 22: Element builder API skeleton for composing styled elements.
- Added RED `element_test` coverage for `ElementBuilder::box()`, fluent
  `style(...)` and `child(...)`, `StyledElement`, retained style data, and
  child ownership; the test failed because the builder/styled element APIs did
  not exist.
- Implemented Step 22 in `codex/element-builder-api`: added `StyledElement`
  and `ElementBuilder` to `element.hpp`, with style storage and optional child
  ownership while leaving paint and padding layout for later steps.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 24/24.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 21/21.
- Started Step 27: Clip rect and overflow primitives.
- Added RED `style_test` coverage for default visible overflow, absent default
  clip rect, fluent `with_overflow(Overflow::hidden)`,
  `with_clip_rect(Rect)`, and the concrete `Style::overflow` type; the test
  failed because those APIs did not exist.
- Implemented Step 27 in `codex/clip-overflow-primitives`: added an inert
  `Overflow` enum, default `Style::overflow = Overflow::visible`, optional
  `Style::clip_rect`, and matching fluent setters.
- Verified targeted tests: `xmake test -P . style_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 24/24.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 21/21.
- Started Step 23: Styled element paints a solid background rect.
- Added RED `element_test` coverage for `StyledElement::paint(PaintList&)`,
  background-color fill rect emission from retained layout bounds, and no-op
  painting when no background color exists; the test failed because
  `Element::paint` did not exist.
- Implemented Step 23 in `codex/styled-background-paint`: added a minimal
  element paint hook and made `StyledElement` fill its retained bounds before
  painting its optional child.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 24/24.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 21/21.
- Started Step 24: Padding participates in styled element layout.
- Added RED `element_test` coverage for styled preferred-size padding,
  child-derived padded size, and child layout bounds offset by top/left padding;
  the test failed because padding was not included in `StyledElement::layout`.
- Implemented Step 24 in `codex/padding-styled-layout`: `StyledElement` now
  adds padding to content size and positions its optional child inside the
  padding inset.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 24/24.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 21/21.
- Started Step 25: Border width/color and border radius primitives.
- Added RED `style_test` coverage for `BorderRadii`, border color storage, and
  fluent border radius/color setters; the test failed because those style APIs
  did not exist.
- Implemented Step 25 in `codex/border-style-primitives`: added `BorderRadii`,
  optional `Style::border_color`, `Style::border_radius`, and matching builder
  methods while keeping rendering/layout behavior unchanged.
- Verified targeted tests: `xmake test -P . style_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 24/24.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 21/21.
- Started Step 26: Paint tree generation from element hierarchy.
- Added RED `element_test` coverage for `ElementTree::paint(PaintList&)`,
  root-first recursive paint traversal, append-order sibling painting, and
  retained element bounds in generated paint commands; the test failed because
  `ElementTree::paint` did not exist.
- Implemented Step 26 in `codex/element-paint-tree`: added a root-recursive
  `ElementTree::paint` traversal that paints each element before its children.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 24/24.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 21/21.

- Started Step 5: Context entity access.
- Added design and implementation plan docs for context-forwarded runtime entity helpers.
- Added a RED runtime test that inserts, reads, mutates, removes, and emplaces an entity from `WindowRuntimeContext`; the test failed because `EntityId` and context entity helpers were not available through the UI runtime API.
- Implemented Step 5 in `codex/context-entity-access`: `WindowRuntime` owns type-erased per-type `EntityStore<T>` instances and `WindowRuntimeContext` forwards typed insert/emplace/read/mutate/remove helpers.
- Fixed the context helper constness so event handlers receiving `const WindowRuntimeContext&` can still mutate runtime-owned entity storage through the controlled capability methods.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default entity_store_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 20/20.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 17/17.
- Started Step 6: View identity allocation.
- Added design and implementation plan docs for runtime-owned `ViewId` allocation.
- Added a RED runtime test for `WindowRuntimeContext::allocate_view_id`, `WindowRuntimeContext::is_view_id_allocated`, monotonic allocation after the root view, and cross-event allocation stability; the test failed because those context APIs did not exist.
- Implemented Step 6 in `codex/view-identity-allocation`: `WindowRuntime` allocates monotonic view ids starting after the root view and recognizes root/allocated ids for the runtime lifetime.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 20/20.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 17/17.
- Started Step 7: ElementId and Element base API.
- Added design and implementation plan docs for the minimal element identity contract.
- Added a RED UI test for `ElementId`, polymorphic `Element`, default invalid ids, and stable assigned ids; the test failed because `cgpui/ui/element.hpp` did not exist.
- Implemented Step 7 in `codex/element-base-api`: `ElementId` is a typed numeric id wrapper and `Element` stores a runtime/tree-assigned id through `id()` and `assign_id()`.
- Added the `element_test` xmake target and included the new header in `ui_header_cleanliness`.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 21/21.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 18/18.
- Started Step 8: Element tree container.
- Added design and implementation plan docs for an owning `ElementTree` with root, children, and parent links.
- Added RED tests for root storage, child order, parent lookup, unknown-parent append rejection, and root replacement; the test failed because `ElementTree` did not exist.
- Implemented Step 8 in `codex/element-tree-container`: `ElementTree` owns elements, assigns monotonic `ElementId` values, stores child lists in append order, and exposes parent/get/children/root lookup helpers.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 21/21.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 18/18.
- Started Step 9: Element reconcile pass.
- Added design and implementation plan docs for a simple rebuild pass that preserves element ids.
- Added RED tests for `ElementTree::reconcile_root`, `ElementTree::reconcile_child`, root id preservation, child id preservation by parent-local index, append-on-new-index behavior, and unknown-parent rejection; the test failed because the reconcile APIs did not exist.
- Implemented Step 9 in `codex/element-reconcile-pass`: root reconcile replaces the root element while preserving id, and child reconcile replaces or appends children by parent/index while preserving existing child ids.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 21/21.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 18/18.
- Started Step 10: LayoutInput/LayoutOutput primitives.
- Added design and implementation plan docs for layout constraints, inputs,
  outputs, and clamping.
- Added a RED UI test for `LayoutInput`, `LayoutConstraints`,
  `constrain_size`, and `LayoutOutput`; the test failed because
  `cgpui/ui/layout.hpp` did not exist.
- Implemented Step 10 in `codex/layout-primitives`: `LayoutConstraints`
  stores min/max sizes, `LayoutInput` carries constraints, `LayoutOutput`
  stores origin/size, and `constrain_size` clamps preferred sizes per axis.
- Added the `layout_test` xmake target and included the new header in
  `ui_header_cleanliness`.
- Verified targeted tests: `xmake test -P . layout_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 11: Layout pass for fixed-size leaf elements.
- Added design and implementation plan docs for a leaf-only element layout
  pass.
- Added a RED UI test for `Element::layout`,
  `FixedSizeElement::layout`, preferred-size output, zero-origin output, and
  constraint clamping; the test failed because those APIs did not exist.
- Implemented Step 11 in `codex/fixed-size-layout`: `Element` now has a
  minimal virtual `layout(LayoutInput)` returning constrained zero size, and
  `FixedSizeElement` returns its constrained preferred size.
- Verified targeted tests: `xmake test -P . element_test/default layout_test/default ui_header_cleanliness/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 14: Pointer routing to hit-tested element id.
- Added design and implementation plan docs for attaching hit-tested
  `ElementId` values to pointer event routes.
- Added a RED runtime test for `EventRoute::target_element_id`,
  `WindowRuntime::set_element_root`, pointer moved/button/scroll hit targets,
  and non-pointer events without element targets; the test failed because the
  routing APIs did not exist.
- Implemented Step 14 in `codex/pointer-hit-routing`: `EventRoute` now carries
  an optional `ElementId`, and `WindowRuntime` hit-tests an installed element
  root for pointer events while preserving root-view delivery.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default element_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 17: Action/Command primitive.
- Added design and implementation plan docs for runtime-local named actions.
- Added a RED runtime test for `ActionDispatchResult`, `ActionHandler`,
  `register_action`, `dispatch_action`, and `last_action_dispatch`; the test
  failed because the action APIs did not exist.
- Implemented Step 17 in `codex/action-command-primitive`: runtime stores
  named action handlers, dispatches them with `WindowRuntimeContext`, records
  handled/missing dispatch results, and exposes the last action dispatch.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 19: Basic text model.
- Added design and implementation plan docs for a minimal editable UTF-8 text
  buffer.
- Added a RED text-model test target for empty defaults, UTF-8 insertion,
  cursor movement, backspace, and forward delete; the test failed because
  `cgpui/ui/text.hpp` did not exist.
- Implemented Step 19 in `codex/text-model`: `TextModel` stores a UTF-8 string
  and byte-offset cursor, inserts at the cursor, moves over codepoint
  boundaries, and supports backspace/forward delete.
- Debugged one test expectation issue: the backspace test moved the cursor to
  offset 0 before expecting another successful backspace, which contradicted
  the intended "delete previous codepoint" behavior.
- Verified targeted tests: `xmake test -P . text_model_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 23/23.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 20/20.
- Started Step 20: Text input routing into focused text model.
- Added design and implementation plan docs for routing `TextInput` into the
  `TextModel` bound to the focused element.
- Added a RED runtime test for `bind_text_model(ElementId, TextModel*)`,
  element keyboard focus, text insertion into the focused model, and ignoring
  text input after focus release; the test failed because `bind_text_model`
  did not exist.
- Implemented Step 20 in `codex/text-input-routing`: runtime stores non-owning
  text model bindings by `ElementId` and inserts `TextInput::text` into the
  model bound to the active element keyboard-focus owner.
- Debugged one test sequencing issue: the release helper triggers on the third
  key event, so the test must send three key events before expecting later text
  input to be ignored.
- Verified targeted tests: `xmake test -P . window_runtime_test/default text_model_test/default ui_header_cleanliness/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 23/23.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 20/20.
- Started Step 18: Key binding table.
- Added design and implementation plan docs for mapping key events to named
  actions.
- Added a RED runtime test for `KeyBinding`, `bind_key`, exact
  key/modifier/action matching, and action dispatch from a matched key event;
  the test failed because the key binding APIs did not exist.
- Implemented Step 18 in `codex/key-binding-table`: runtime stores key
  bindings, matches `KeyboardKey` events by key code/action/modifiers, and
  dispatches the first matching named action.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 15: Pointer capture routing to owner id.
- Added design and implementation plan docs for element-level pointer capture
  overriding hit-tested pointer routes.
- Added a RED runtime test for `ViewInputState::pointer_capture_element_owner`,
  `capture_pointer(ElementId)`, owner-matched release, wrong-owner release, and
  capture overriding hit-tested `EventRoute::target_element_id`; the test failed
  because the element capture APIs did not exist.
- Implemented Step 15 in `codex/pointer-capture-routing`: runtime stores an
  optional captured `ElementId`, exposes it through input state, routes pointer
  events to it before hit testing, and releases it only when the owner matches.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default element_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 16: Keyboard routing to focus owner id.
- Added design and implementation plan docs for element-level keyboard focus
  routing.
- Added a RED runtime test for `ViewInputState::keyboard_focus_element_owner`,
  `request_keyboard_focus(ElementId)`, owner-matched release, wrong-owner
  release, and keyboard/text `EventRoute::target_element_id`; the test failed
  because the element keyboard focus APIs did not exist.
- Implemented Step 16 in `codex/keyboard-focus-routing`: runtime stores an
  optional focused `ElementId`, exposes it through input state, routes keyboard
  and text input events to it, and releases it only when the owner matches.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default element_test/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 12: Layout pass for simple vertical stack.
- Added design and implementation plan docs for a simple owned-child vertical
  stack element.
- Added a RED UI test for `VerticalStackElement`, child ownership, empty-stack
  constrained size, max-child-width layout, summed-child-height layout, and
  stack-level constraint clamping; the test failed because
  `VerticalStackElement` did not exist.
- Implemented Step 12 in `codex/vertical-stack-layout`: `VerticalStackElement`
  owns child elements, ignores null appends, exposes read-only children, and
  computes layout as max child width plus summed child heights.
- Verified targeted tests: `xmake test -P . element_test/default layout_test/default ui_header_cleanliness/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 13: Hit testing over laid-out element bounds.
- Added design and implementation plan docs for retained element bounds and
  element-local hit testing.
- Added RED UI tests for `layout_bounds()`, leaf `hit_test(Point)`, vertical
  stack child bounds, and child-before-self stack hit testing; the test failed
  because the hit-testing APIs did not exist.
- Implemented Step 13 in `codex/hit-testing`: elements retain their latest
  layout bounds, `FixedSizeElement` and `VerticalStackElement` record bounds
  during layout, and stack hit testing checks children before falling back to
  the stack id.
- Verified targeted tests: `xmake test -P . element_test/default layout_test/default ui_header_cleanliness/default` passed 3/3.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 22/22.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 19/19.
- Started Step 31: Scroll state/model primitive.
- Added RED `scroll_test` coverage for `ScrollModel` default state, clamped
  offsets, reclamping after viewport/content size changes, and
  `can_scroll_x`/`can_scroll_y`; the test failed because
  `cgpui/ui/scroll.hpp` did not exist.
- Implemented Step 31 in `codex/scroll-state-model`: added a header-only
  `ScrollModel` with viewport/content sizes, clamped offset state, relative
  scrolling, and axis scrollability checks.
- Verified targeted tests: `xmake test -P . scroll_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 25/25.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 22/22.
- Started Step 35: View/model subscription relation skeleton.
- Added RED `window_runtime_test` coverage for typed view-to-entity
  subscriptions, subscription lookup by view, entity-change notification,
  missing notification misses, and subscription-triggered layout invalidation;
  the test failed because subscription APIs did not exist.
- Implemented Step 35 in `codex/view-model-subscription`: added
  `EntitySubscription`, typed `subscribe_view_to_entity`,
  `subscriptions_for_view`, and `notify_entity_changed`, with notification
  requesting layout through the existing invalidation/scheduling path.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 25/25.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 22/22.
- Started Step 36: Text selection range model.
- Added RED `text_model_test` coverage for collapsed default selections,
  anchor/head range storage, normalized ranges, clamped offsets, explicit
  selection clearing, cursor movement collapsing selection, and insertion
  replacing the selected range; the test failed because selection APIs did not
  exist.
- Implemented Step 36 in `codex/text-selection-range`: added
  `TextSelectionRange`, selection anchor/head accessors, normalized
  `selection()`, `set_selection`, `clear_selection`, cursor-collapse behavior,
  and selected-range replacement on insertion.
- Verified targeted tests: `xmake test -P . text_model_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 25/25.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 22/22.
- Started Step 37: Text key editing actions for delete, movement, and
  selection.
- Added RED `text_model_test` coverage for `TextEditAction`,
  `apply_edit_action`, previous/next movement, selection extension,
  selection-aware delete/backspace, and boundary misses; the test failed
  because edit-action APIs did not exist.
- Implemented Step 37 in `codex/text-edit-actions`: added a small
  `TextEditAction` enum, an `apply_edit_action` dispatcher, selection-extending
  movement helpers, and selection-aware delete/backspace behavior.
- Verified targeted tests: `xmake test -P . text_model_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 25/25.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 22/22.
- Started Step 38: Clipboard abstraction for Win32 and Wayland.
- Added RED `clipboard_test` coverage for a text clipboard contract,
  `MemoryClipboard`, UTF-8 round trips, and `create_platform_clipboard`; the
  test failed because `cgpui/platform/clipboard.hpp` did not exist.
- Implemented Step 38 in `codex/clipboard-abstraction`: added the platform
  `Clipboard` interface, `MemoryClipboard`, a platform clipboard factory, a
  `clipboard_test` target, and header-cleanliness coverage.
- Verified targeted tests: `xmake test -P . clipboard_test/default core_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 26/26.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 23/23.
- Started the next 20-step continuation from Step 40 and extended
  `task_plan.md` with Steps 41-60 toward a closer GPUI core API surface.
- Started Step 40: Real interactive demo using the new API surface.
- Added RED `hello_window_lifetime_test` coverage requiring the demo source to
  use `ElementTree`, `ElementBuilder`, `TextModel`, runtime element root
  installation, text-model binding, cursor binding, actions, key bindings,
  keyboard focus, view-model subscriptions, entity-change notification,
  layout invalidation, and scripted text injection; the test failed because
  the demo still painted a hard-coded rect.
- Implemented Step 40 in `codex/new-api-demo`: `HelloView` now owns an element
  tree and text model, paints through the element tree, binds runtime text and
  cursor state, registers a clear-text action/key binding, requests element
  keyboard focus, subscribes to a demo entity, notifies model changes, and
  supports `CGPUI_DEMO_INJECT_TEXT` for smokeable state changes.
- Verified targeted tests: `xmake test -P . hello_window_lifetime_test/default
  hello_window/windows_first_frame hello_window/windows_resize_after_first_frame
  hello_window/windows_close_after_first_frame ui_header_cleanliness/default`
  passed 5/5.
- Started Step 41: ElementTree root layout helper.
- Added RED `element_test` coverage for `ElementTree::layout_root`, including
  root layout delegation, retained root bounds, and empty-tree constrained zero
  output; the test failed because `layout_root` did not exist.
- Implemented Step 41 in `codex/element-tree-layout-root`: added a header-only
  `ElementTree::layout_root(LayoutInput)` helper that delegates to the root
  element when present and returns constrained zero size for an empty tree.
- Verified targeted tests: `xmake test -P . element_test/default
  ui_header_cleanliness/default` passed 2/2.
- Started Step 43: Element tree owned runtime root installation.
- Added RED `window_runtime_test` coverage for
  `WindowRuntime::set_element_tree`, `element_tree`, and `element_root`,
  including move ownership, pointer routing through the owned tree, and clearing
  the owned tree; the test failed because the runtime-owned tree APIs did not
  exist.
- Implemented Step 43 in `codex/runtime-owned-element-tree`: runtime can now
  own an `ElementTree`, exposes read-only accessors, clears legacy non-owning
  roots when an owned tree is installed, clears owned trees when a legacy root
  is installed, and routes pointer hit testing through the owned tree first.
- Verified targeted tests: `xmake test -P . window_runtime_test/default
  ui_header_cleanliness/default` passed 2/2.
- Started Step 45: Element event handler hook.
- Added RED `element_test` coverage for `ElementEventContext`, a default
  unhandled `Element::handle_event`, and `StyledElement` forwarding events to
  its child; the test failed because element-level event APIs did not exist.
- Implemented Step 45 in `codex/element-event-handler-hook`: moved
  `EventResult` into the element-visible API surface, added
  `ElementEventContext`, added the virtual element event hook, and made
  `StyledElement` forward events to its child.
- Verified targeted tests: `xmake test -P . element_test/default
  ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests on `master` after merging Step 45:
  `xmake f -c -m debug -P .; xmake test -P .` passed 26/26.
- Verified WSL Arch Linux full debug tests on `master` after merging Step 45:
  `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .`
  passed 23/23.
- Started Step 46: Runtime dispatches routed element events before view
  fallback.
- Added RED `window_runtime_test` coverage for routed owned-tree element event
  handlers: consumed element events skip view fallback, unhandled element
  events fall back to the view, and dispatch records preserve the routed
  element id plus final result; the test failed because runtime did not call
  element handlers.
- Implemented Step 46 in `codex/runtime-element-event-dispatch`: runtime now
  resolves the routed element id, calls the element handler before view
  fallback, skips the view when the element consumes/cancels, and records the
  actual dispatch result.
- Verified targeted tests: `xmake test -P . window_runtime_test/default
  ui_header_cleanliness/default` passed 2/2.
- Started Step 44: Runtime lays out installed element trees on redraw.
- Added RED `window_runtime_test` coverage for an owned element tree that is not
  pre-laid-out before runtime redraw; the test expected redraw to lay out the
  root with viewport constraints and later pointer routing to hit the root, and
  failed because runtime redraw did not lay out owned trees.
- Implemented Step 44 in `codex/runtime-layout-owned-tree`: `handle_redraw`
  now lays out the owned element tree root with the current viewport as the max
  layout constraint before rendering the view.
- Verified targeted tests: `xmake test -P . window_runtime_test/default
  ui_header_cleanliness/default` passed 2/2.
- Started Step 42: ElementTree root hit-test helper.
- Added RED `element_test` coverage for `ElementTree::hit_test_root`, including
  root hit delegation, miss behavior, and empty-tree invalid hits; the test
  failed because `hit_test_root` did not exist.
- Implemented Step 42 in `codex/element-tree-hit-test-root`: added a
  header-only `ElementTree::hit_test_root(Point)` helper that delegates to the
  root element and returns invalid when the tree is empty.
- Verified targeted tests: `xmake test -P . element_test/default
  ui_header_cleanliness/default` passed 2/2.
- Started Step 39: IME composition skeleton for Win32 and Wayland.
- Added RED `text_model_test` coverage for text-model composition update,
  commit, and cancel; the test failed because composition APIs did not exist.
- Added RED `window_runtime_test` coverage for `ImeComposition`,
  `ImeCompositionPhase`, `EventKind::ime_composition`, keyboard-focus routing
  of composition events, focused text-model preedit update, and commit
  insertion; the test failed because the event/model APIs did not exist.
- Implemented Step 39 in `codex/ime-composition-skeleton`: added
  `ImeComposition` platform events, `ime_composition` routing kind, text-model
  composition state, and runtime routing from focused IME events into bound text
  models.
- Verified targeted tests: `xmake test -P . text_model_test/default window_runtime_test/default ui_header_cleanliness/default core_header_cleanliness/default` passed 4/4.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 26/26.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 23/23.
- Started Step 34: Update scheduling that requests redraw after model/view
  changes.
- Added RED `window_runtime_test` coverage for invalidation-triggered redraw
  scheduling, duplicate request coalescing during one event, and clearing dirty
  state after the redraw frame is consumed; the test failed because
  invalidation did not request platform redraws.
- Implemented Step 34 in `codex/update-scheduling-redraw`: invalidation
  requests now schedule a single deferred redraw during view event dispatch,
  initial redraw marks a frame as scheduled, and successful redraw clears
  invalidation and scheduling state.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 25/25.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 22/22.
- Started Step 33: Runtime invalidation requests for layout and paint.
- Added RED `window_runtime_test` coverage for runtime layout/paint
  invalidation state, layout requests implying paint invalidation, explicit
  paint requests, clear behavior, and after-event observability; the test
  failed because `InvalidationState` and invalidation APIs did not exist.
- Implemented Step 33 in `codex/runtime-invalidation`: added
  `InvalidationState`, `request_layout`, `request_paint`,
  `clear_invalidation`, and `invalidation_state` on `WindowRuntime`.
- Verified targeted tests: `xmake test -P . window_runtime_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 25/25.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 22/22.
- Started Step 32: Flex row/column layout basics.
- Added RED `element_test` coverage for `FlexDirection`, `FlexElement`, row
  left-to-right layout, column top-to-bottom layout, child bounds, constraints,
  and child-first hit testing; the test failed because the flex APIs did not
  exist.
- Implemented Step 32 in `codex/flex-layout-basics`: added a minimal
  `FlexElement` with owned children, row/column measurement, retained child
  bounds, and child-first hit testing.
- Verified targeted tests: `xmake test -P . element_test/default ui_header_cleanliness/default` passed 2/2.
- Verified Windows full debug tests: `xmake f -c -m debug -P .; xmake test -P .` passed 25/25.
- Verified WSL Arch Linux full debug tests: `XMAKE_ROOT=y xmake f -y -c -m debug -P . && XMAKE_ROOT=y xmake test -y -P .` passed 22/22.
