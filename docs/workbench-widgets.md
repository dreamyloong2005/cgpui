# Workbench widgets (C9)

Splitter, tab strip, tree view, tooltip and popover / context menu for an
IDE workbench. Headers: `include/cgpui/ui/{splitter,tab_strip,tree_view,
tooltip,popover_menu,anchored,widget_tree,workbench_theme}.hpp`, aggregate
`workbench_widgets.hpp`. Sources: `src/ui/workbench/` (model / layout /
input / paint split per widget). Build module:
`build/xmake/capabilities/workbench_widgets_targets.lua`.

## Integration model

Every widget is an app-owned object with the same four calls:

| Call | When | Does |
|---|---|---|
| `layout(bounds, scale, ...)` | in `View::render` | geometry in window logical px, edges on device pixels |
| `append_elements(builder, parent)` | in `View::render` | routing / focus / accessibility nodes |
| `handle_event(event, context)` | in `View::handle_event` | input; returns `EventResult` |
| `paint(paint_list, theme)` | in `View::paint` | draws with `WorkbenchTheme` |

`WidgetTreeBuilder` builds the window element tree from `WidgetNode`s
(role, name, value, `AccessibilityPatternState`, bounds, focusable,
hit-testable, z order, cursor). `install(context)` sets it with
`WindowRuntime::set_element_tree` (render returns an empty element), applies
node cursors and a pending focus request. Nodes marked *stable* get their ids
first, so a container keeps its id when rows, tabs or menu items change;
keyboard focus, pointer capture and cursors key on those ids.
Widgets give node bounds in window coordinates; the builder stores each
node's offset from its parent node and the element layer places it back
(`ElementTree::layout_root` accumulates parent origins, as GPUI's layout
bounds are absolute). `ElementTree::hit_test_root` routes the hit: tree
children before parents, higher z order first, later siblings first.

Widget nodes never consume events, so every event reaches the view. The app
forwards in this order: popover menu (modal while open), tooltip, its own
title-bar controls, splitter, tabs, tree. See `examples/workbench/main.cpp`.

Widgets report changes through one handler each and never change app-owned
data: the splitter reports fractions, the tab strip reports select / close /
reorder, the tree reports expansion, selection, activation, child requests and
context-menu requests, the menu reports the activated item id.

## Splitter

```cpp
Splitter(SplitterOptions options, std::vector<SplitterPaneOptions> panes);
void layout(Rect bounds, DpiScale scale);
Rect pane_rect(std::size_t pane) const;
std::span<const float> fractions() const;            // persisted by the app
void set_fractions(std::span<const float> fractions); // restore
void set_collapsed(std::size_t pane, bool collapsed);
void on_change(SplitterChangeHandler handler);        // SplitterChange{fractions, divider, source, finished}
bool move_divider(std::size_t divider, float delta, SplitterChangeSource source, bool finished = true);
```

- `SplitterAxis::horizontal` (panes left to right) or `vertical`, two or
  more panes, `min_size` / `max_size` / `collapsible` per pane.
- Divider line `divider_thickness` (1 logical px, at least one device pixel)
  with a centered hit area of `hit_thickness` (8 px), resize cursor via the
  node cursor, pointer capture on the divider node during a drag, and a
  window-sized shield node while dragging so the cursor stays.
- A collapsible pane dragged below half its minimum collapses; dragging back
  expands it. Double-click restores the default fractions.
- Keyboard on a focused divider: arrows +-16 px, Shift+arrows +-64 px,
  Home / End to the limits, Enter toggles the adjacent collapsible pane.
- Drags report `finished = false` per move and `finished = true` on release.
- Accessibility: each divider is a focusable `separator` with a 0..100 range
  value.

## Tab strip

```cpp
void set_tabs(std::vector<TabItem> tabs);  // TabItem{id, title, dirty, closable}
void set_selected(std::optional<TabId> tab);
void on_event(TabStripEventHandler handler); // select / close / reorder(from, to)
void layout(Rect bounds, DpiScale scale, const WorkbenchTheme& theme);
```

- Tab width = padding + measured title + close slot, clamped to
  72..240 px. Titles are measured once with `measure_text` (native shaping,
  C7) and re-measured only when titles, font or scale change.
- Overflow: the strip scrolls horizontally (wheel: 40 px per notch, precise
  deltas in px) and scrolls the selected tab into view. There is no overflow
  menu.
- Dirty slot: a dot in the close position; on hover it becomes the close
  button. Left click selects on press; the close button closes on release;
  middle click (press and release on the same tab) closes.
- Drag to reorder within one strip: 5 px threshold, insertion marker, the
  dragged tab follows the pointer with a shadow; release reports
  `reorder(from, to)`. Dragging tabs to another strip or window is out of
  scope.
- Keyboard on the focused strip: Left / Right / Home / End select. Ctrl+Tab
  and Ctrl+Shift+Tab belong to the app key bindings / actions.
- Accessibility: `tab_list` (focusable) with `tab` children (`selected`,
  value "modified" when dirty) and a `button` "Close <title>" per tab.

## Tree view

```cpp
void set_roots(std::vector<TreeItem> roots);                   // TreeItem{id, label, has_children}
void set_children(TreeItemId parent, std::vector<TreeItem> children);
void expand(TreeItemId); void collapse(TreeItemId);
void select(TreeItemId, TreeSelectMode mode = TreeSelectMode::replace); // replace / toggle / extend
std::vector<TreeItemId> selection() const;
UniformListVisibleRange visible_range() const;
void on_event(TreeViewEventHandler handler);
```

- Virtualized: rows are a flattened id list rebuilt only when expansion or
  children change; only `visible_range()` (a `UniformListVisibleRange`,
  viewport rows plus `overscan` = 4) gets nodes and paint. With 100 000
  children the tree produces 24 row nodes at the top and 28 in the middle.
- Lazy children: expanding an unloaded item reports `children_requested`;
  the app answers with `set_children` now or later.
- Stable identity: items are keyed by `TreeItemId`; `set_children` keeps
  nodes whose id survives (expansion, loaded children, selection) and drops
  removed subtrees from selection and focus.
- Pointer: click selects; Ctrl+click toggles; Shift+click extends from the
  anchor; the twisty or a double-click toggles expansion; double-click on a
  leaf activates; right-click selects (keeping a multi-selection) and reports
  `context_menu_requested` with the position. Pointer events carry no
  modifiers, so the tree tracks Shift / Ctrl from keyboard events.
- Keyboard: Up / Down (Shift extends), Page Up / Down, Home / End, Left
  collapses or goes to the parent, Right expands or goes to the first child,
  Enter activates, Ctrl+Space toggles, Apps or Shift+F10 requests the
  context menu. Type-to-select: typed text forms a prefix matched ASCII
  case-insensitively; it resets after `type_ahead_timeout_ms` (1000) through a
  runtime timer that only holds a weak reference.
- Indentation guides are optional (`indent_guides`).
- Accessibility: `tree` (focusable) with `tree_item` children carrying
  `level` (1-based), `expanded` (items with children) and `selected`.

## Tooltip

```cpp
Tooltip(TooltipOptions options = {});  // delay_ms 500, fade_ms 120, position pointer | anchor
void set_target(TooltipTargetId id, Rect bounds, std::string_view text);
void layout(Size viewport, DpiScale scale, const WorkbenchTheme& theme);
```

- Shows after `delay_ms` over a target (runtime timer; tests drive it with
  `TestApp::advance_time`). Hides on leave, press, scroll (until the pointer
  leaves the target), Escape (consumed only while visible), pointer exit and
  window blur.
- Position: below the pointer, or below the target element; flips above when
  there is no room and shifts horizontally to stay 4 px inside the window.
- The fade uses `start_animation`; it is skipped when
  `WorkbenchTheme::reduced_motion` is set (from
  `SystemAppearance::client_area_animation_enabled`, C2).
- Single line, clipped at `max_width` (360 px); no wrapping.
- Accessibility: a non-focusable `tooltip` node while visible.

## Popover / context menu

```cpp
void open_at(Point position, std::vector<MenuItem> items, const WindowRuntimeContext&);
void open_beside(Rect anchor, AnchoredSide side, std::vector<MenuItem> items, const WindowRuntimeContext&);
void close(PopoverCloseReason reason, const WindowRuntimeContext&);
void on_activate(MenuActivateHandler handler);  // item id
void on_close(PopoverCloseHandler handler);     // activated / escape / outside_click / focus_lost / app
// MenuItem::action(id, label, accelerator, enabled), separator(), submenu(label, children)
```

- Overlay layer (GPUI `deferred` + `anchored`): the menu root node is always
  in the tree (stable id); while open it covers the window at z 1000, takes
  keyboard focus and every pointer hit, and the menu paints last. A press
  outside every level closes it (`outside_click`) and is consumed.
- Placement (`anchored.hpp`, GPUI names): `anchored_bounds` puts an
  `AnchorCorner` on a point with `AnchoredFitMode::switch_anchor` (flip the
  corner per overflowing axis, then shift) or `snap_to_window`;
  `anchored_bounds_beside` opens on an `AnchoredSide` of a rect and flips to
  the opposite side. Submenus open to the right of their item and flip left.
- Focus: opening saves the focused element and focuses the menu node;
  closing restores the saved element (or releases focus).
- Keyboard: Up / Down / Home / End skip separators and disabled items and
  wrap; Right or Enter opens a submenu; Left or Escape closes a submenu;
  Escape on the top level closes; Enter / Space activate; Tab closes. All
  keys are consumed while open. Hover highlights and opens submenus at once;
  activation happens on a left release after a press inside the menu.
- Accelerator column: labels and accelerators are measured with
  `measure_text`; the column starts after the widest label + 32 px and
  accelerators are right-aligned in it.
- Surface: C5 `draw_box_shadow` (24 px blur, 8 px offset, radius 8) and
  optional C6 `draw_backdrop_blur` (`PopoverMenuOptions::backdrop_blur`).
- Accessibility: `menu` (focusable) with `menu_item` children (`enabled`,
  value = accelerator, `selected` = highlighted, `expanded` on submenu items)
  and `separator`; an open submenu is a `menu` child of its item.
- Limitation: menus are in-window overlays and cannot extend outside the
  window. Native popups (`xdg_popup`, Win32 popup windows, macOS NSMenu) are
  not used; the public API does not depend on them, so the macOS plan C9 row
  is unchanged.

## Theming

`workbench_theme(SystemAppearance, const Theme* overrides)` picks light or
dark defaults from the system mode (C2), the accent color when the system
reports one (else `#0078D4`), black / white high-contrast colors with a 2 px
focus ring, and `reduced_motion`. Any color can be overridden with a theme
token `workbench.<field>`: surface, surface_alt, text, text_muted,
text_disabled, accent, accent_text, border, divider, hover, selection,
selection_inactive, focus_ring, tab_strip, tab_active, dirty, indent_guide,
tooltip, tooltip_text, menu, menu_border, menu_highlight,
menu_highlight_text, shadow. Focus rings: divider accent band, outline on the
selected tab, outline on the focused tree row.

## Accessibility mapping

New `AccessibilityRole` values: `tab_list`, `tab`, `tree`, `tree_item`,
`menu`, `separator`, `tooltip`. New `AccessibilityPatternState` fields:
`expanded`, `selected`, `level` (mirrored in `PlatformAccessibility*`).

| Role | UIA control type | AT-SPI role |
|---|---|---|
| tab_list / tab | Tab / TabItem | page tab list (38) / page tab (37) |
| tree / tree_item | Tree / TreeItem | tree (65) / tree item (91) |
| menu / menu_item | Menu / MenuItem | menu (33) / menu item (35) |
| separator | Separator | separator (50) |
| tooltip | ToolTip | tool tip (64) |

AT-SPI exposes `expandable` + `expanded` / `collapsed` and `selectable` +
`selected` states. UIA exposes `UIA_LevelPropertyId`; the UIA ExpandCollapse
and SelectionItem patterns are not implemented, so expanded / selected state
is not visible to UIA clients yet.

## Allocation

Steady state: widget layout, hit testing (`ElementTree::hit_test_root` walks z
levels without sorting), input and paint do not allocate; text is measured when it
changes. Each render builds a new element tree (`WidgetTreeBuilder`
allocates nodes and accessibility strings), which is the framework render
model. `PaintList::fill_text` copies the string per paint command.

## Limits

- Record windows (TestApp windows and additional app windows) keep no
  dynamic element tree in the runtime: `install` builds the tree for ids but
  only the root window publishes it for hit testing, cursors and the platform
  accessibility tree. Widget input does not depend on it, since the view
  forwards every event. Behavior tests read accessibility through
  `WidgetTreeBuilder::finish()` and hit testing through
  `ElementTree::hit_test_root`.
- Text in tabs, tree rows, tooltips and menus is single-line and clipped,
  with no ellipsis.
- Menus cannot leave the window; no native popup windows.
- No cross-strip or cross-window tab drag.
- The tree keys rows by id and assumes each id appears once; moving an item
  to another parent with the same id is not supported.

## Screenshot observations (Windows, 150%, 1280 x 800 logical)

Captured with `D:\Dev\shots\shot.ps1` and `shot_workbench.ps1` (real
`SetCursorPos` / `mouse_event` input) from `example_workbench`.

- `c9_workbench.jpg` (dark, system mode): client title bar 36 px with title,
  panel-toggle icon and minimize / maximize / close. Three panes at about
  20 / 55 / 25 %: explorer with a tree of 300+ rows (12 folders x 25 files),
  indent guides and triangle twisties; editor with six tabs, `main.cpp`
  selected with an accent top line and a close glyph, `workbench.hpp` showing
  the dirty dot; agent panel with three message cards. Text is sharp
  (Segoe UI, monospace editor), dividers are 2 device pixels.
- `c9_workbench_menu.jpg`: right-click on a tree row selects it and opens the
  context menu at the pointer: soft shadow below and right, rounded surface,
  separators, a disabled "Delete", right-aligned accelerators
  (Ctrl+N, Shift+Alt+R, Shift+Alt+C, F2, Del) in one column, a submenu arrow.
- `c9_workbench_tooltip_light.jpg` (light, forced): hovering the title-bar
  panel button shows "Toggle agent panel (Ctrl+Alt+B)" below the pointer,
  shifted left to stay inside the window; light theme colors throughout.

## Not verified

- Splitter drag, tab drag and keyboard focus rings in a real window (covered
  by behavior tests only); the resize cursor through the root window hit test.
- Narrator / Accessibility Insights and Orca reading the new roles.
- Backdrop blur under the menu (`CGPUI_WORKBENCH_BLUR=1`) and Mica.
- Wayland: built and tested headless on Linux; no Wayland window captured.
- High contrast and accent colors from the real system setting.
