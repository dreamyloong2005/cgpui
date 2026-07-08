# Public Authoring Vocabulary Freeze

Phase B Step 308 established the initial public authoring vocabulary freeze.
Phase B Steps 313-318 close the final Phase B freeze that Windows/Linux
examples and API-parity tests may rely on before Phase C starts.

The frozen include boundary is `#include "cgpui/prelude.hpp"`.

This document is a compatibility guard, not a promise that every listed type is
feature-complete with upstream GPUI. It records the public names that should be
used from application-author code and the module boundary that future slices
must keep green. Phase C starts from this frozen boundary and may expand the
element/style/widget vocabulary only through focused public modules.

## Application entry vocabulary

- `Application`
- `App`
- `Window`
- `WindowOptions`
- `AppContext`

These names are available through the prelude and the thin app aggregate. Public
examples should use them instead of constructing runtime internals directly.

## View authoring vocabulary

- `Context<T>`
- `Render<T>`
- `IntoElement`
- `ViewContext`
- `ViewContextCapability<T>`
- `WindowContextCapability`
- `ElementContextCapability`

These names define the author-facing render and context vocabulary. The prelude
is the public include entry; examples should not include focused UI leaves
directly just to access these names.

## Entity and view handle vocabulary

- `EntityHandle<T>`
- `WeakEntity<T>`
- `ViewHandle<T>`
- `WeakViewHandle<T>`

These names are the public typed-handle spelling for Phase B. They wrap the
current runtime storage boundaries without requiring application code to name
runtime ids or stores.

## Action and key vocabulary

- `Action<T>`
- `CommandPaletteEntry`
- `KeyBindingContext`

Typed actions, command-palette metadata, and key-binding context selection are
public authoring vocabulary. Macro-style action payload generation remains
outside this freeze.

## Async, test, and Result vocabulary

- `Result<T>`
- `ErrorCode`
- `AsyncContextCapability`
- `TestContextCapability`
- `try_spawn_task(...) -> Result<TaskHandle>`
- `try_spawn_background_task(...) -> Result<TaskHandle>`
- `run_until_parked()`
- `advance_time_until_parked(...)`
- `try_draw_frame() -> Result<void>`

Phase B public examples may use Result-returning async and test helpers. This
freeze does not add task priorities, structured task groups, or upstream test
macro equivalents.

## Platform service vocabulary

- `NativeMenuModel`
- `NativeFileDialogOptions`

Platform-service vocabulary stays public through Result-returning menu and file
dialog helpers. Production-depth native platform behavior remains later work.

## Built-in widget vocabulary

- `button(...)`
- `ButtonBuilder::label(...)`
- `label(...)`
- `text_input(...)`
- `TextModel`
- `checkbox(...)`
- `radio(...)`
- `toggle_switch(...)`
- `ToggleBuilder`
- `slider(...)`
- `SliderBuilder`
- `list_item(...)`
- `menu_item(...)`
- `ItemBuilder`
- `image(...)`
- `icon(...)`
- `ImageBuilder`
- `div()`
- `h_flex()`
- `v_flex()`
- `h_stack()`
- `v_stack()`

Phase C Step 343 starts the built-in widget surface with button, label, and
text-input builder spelling. Phase C Step 344 adds checkbox, radio, and switch
builder spelling with checked/selected/on state metadata. Phase C Step 345 adds
slider builder spelling with range, value, step, and change-handler metadata.
Phase C Step 346 adds list item and menu item builder spelling with focused
item element action, selection, disabled, and accessibility metadata.
Phase C Step 347 adds image and icon builder spelling over existing image asset
descriptors, source-rect cropping, alternate text, and optional icon tint
metadata.
Phase C Step 348 adds container primitive free-function spelling through a
focused container builder leaf.
Widget implementation bodies live under focused `src/ui/widgets/*` sources
rather than broad UI entry files.

## Uniform list vocabulary

- `UniformListVisibleRange`
- `UniformListItemIdentity`
- `UniformListItemMeasurement`
- `UniformListItemMeasurementResult`
- `UniformListRecyclingWindow`
- `UniformListSelectionSource`
- `UniformListSelectionDirection`
- `UniformListSelection`
- `UniformListSelectionState`
- `UniformListLayoutSnapshot`
- `UniformListScrollAnchor`
- `UniformListItemMeasurementCache`
- `calculate_uniform_list_visible_range(...)`
- `measure_uniform_list_items(...)`
- `calculate_uniform_list_recycling_window(...)`
- `select_uniform_list_item_at_point(...)`
- `move_uniform_list_selection(...)`
- `capture_uniform_list_scroll_anchor(...)`
- `apply_uniform_list_scroll_anchor(...)`
- `ScrollableListElement::layout_snapshot()`
- `ScrollableListElement::measurement_cache()`
- `ScrollableListElement::selection()`

Phase C Step 349 starts uniform list parity with stable item identity records
and a virtualized visible-range calculation boundary. Phase C Step 350 adds
keyed scroll anchoring over that snapshot boundary so a visible item keeps its
viewport offset after preceding item size changes. Phase C Step 351 adds keyed
item measurement caching and exposes cache hit/miss stats from the scrollable
list layout boundary. Phase C Step 352 adds a retained/recycled window over the
visible range plus overscan so scrollable lists can skip painting items outside
the recycled large-list window. Phase C Step 353 adds pointer hit selection,
keyboard previous/next/first/last movement, and snapshot selected-state
marking through focused uniform-list selection helpers. Phase C Step 354 closes the uniform-list band with
`tests/api_parity/phase_c_uniform_list_audit_test.cpp`, freezing the focused
uniform-list public vocabulary and source ownership evidence before Step 355
starts the window/examples widget band.

## Window/examples widget vocabulary

- `WindowOptions`
- `NativeMenuModel`
- `NativeMenuItem`
- `NativeMenuItemKind::submenu`
- `NativeMenuItemKind::separator`
- `NativeMenuAccelerator`
- `CommandPaletteEntry`
- `KeyBindingContext`
- `TextModel`
- `text_input(...)`
- `menu_item(...)`
- `button(...)`
- `label(...)`
- `checkbox(...)`
- `radio(...)`
- `toggle_switch(...)`
- `slider(...)`
- `list_item(...)`
- `image(...)`
- `icon(...)`
- `h_stack()`
- `v_stack()`
- `ImageAsset`
- `ImageAssetId`
- `DecodedImageBitmap`
- `describe_image_asset(...)`
- `BoxShadow`
- `ElementBuilder::shadow(...)`
- `ElementBuilder::shadow_sm()`
- `ElementBuilder::fixed()`
- `ElementBuilder::top(...)`
- `ElementBuilder::left(...)`

Phase C Step 355 window/examples widgets adds
`examples/api_parity/public_window_examples/main.cpp` and
`api_parity_public_window_examples` as the public API entry for menu demos,
shadow, window positioning, window shadow, and input examples. The example is
guarded by `tests/api_parity/phase_c_window_examples_public_api_test.cpp` and
must stay prelude-only.

Phase C Step 356 window/examples workflow adds
`examples/api_parity/public_window_examples_workflow/main.cpp` and
`api_parity_public_window_examples_workflow` as the public test-context workflow
entry for menu installation, window activation/focus, key binding simulation,
pointer dispatch, text input, and shadow/fixed positioning examples. It is
guarded by `tests/api_parity/phase_c_window_examples_workflow_test.cpp` and
must stay prelude-only.

Phase C Step 357 window/examples widget catalog adds
`examples/api_parity/public_window_examples_widget_catalog/main.cpp` and
`api_parity_public_window_examples_widget_catalog` as the public widget catalog
entry for checkbox/radio/switch, slider, list/menu, image/icon, and container
widgets inside a window example. This keeps container widgets visible from the
public window example band. It is guarded by
`tests/api_parity/phase_c_window_examples_widget_catalog_test.cpp` and must
stay prelude-only. Step 358 should continue the window/examples widget band.

Phase C Step 358 window/examples interaction states adds
`examples/api_parity/public_window_examples_interaction_states/main.cpp` and
`api_parity_public_window_examples_interaction_states` as the public interaction
state entry for hover/focus/active/disabled interaction states, focus ring and tab-index examples,
keyed controls, and click handlers inside a window example.
It is guarded by
`tests/api_parity/phase_c_window_examples_interaction_states_test.cpp` and must
stay prelude-only.

Phase C Step 359 window/examples service matrix adds
`examples/api_parity/public_window_examples_service_matrix/main.cpp` and
`api_parity_public_window_examples_service_matrix` as the public service-matrix
entry for menu accelerators and command palette service matrix plus
window options, shadow, fixed positioning, and text input service examples.
It is guarded by `tests/api_parity/phase_c_window_examples_service_matrix_test.cpp`
and must stay prelude-only.

Phase C Step 360 window/examples closeout adds
`tests/api_parity/phase_c_window_examples_closeout_test.cpp` to guard the
Steps 355-359 public window/example evidence, keep the examples prelude-only,
and close the window/examples widget band before Phase C Step 361 SVG/image element front-end APIs.

Phase C Step 361 SVG/image front-end source APIs adds `ImageSource`,
`ImageSourceKind`, `image_source(...)`, `svg_image_source(...)`, and
`svg(...)` as the public source boundary for raster image descriptors and SVG
source strings. `ImageBuilder` continues to expose `image(...)`, `icon(...)`,
and `svg(...)`, and `ImageElement` preserves SVG source metadata without
placing renderer upload details in the public element API. Phase C Step 362
SVG/image asset registration is the next handoff.

Phase C Step 362 SVG/image asset registration adds `ImageAssetRegistry`,
`RegisteredImageAsset`, `register_image(...)`, and `register_svg(...)` as the
public registration boundary for raster image assets and SVG source strings.
Registrations produce `ImageSource` values that feed `image(...)` and `svg(...)`
without adding SVG decoding, renderer upload, or GPU lifetime details to public
element APIs. Phase C Step 363 SVG/image public example coverage is the next
handoff.

## Out of scope for this freeze

- `ClipboardItem` payload parity
- upstream `gpui::test` macro equivalents
- action macro payloads
- task priorities
- structured task groups
- private runtime headers
- direct `WindowRuntime` use

These items remain deferred Phase C or production-depth work and should not be
introduced while freezing the current public vocabulary.

## Forbidden from public examples

Public compatibility examples must not include private headers, source-relative
headers, platform or renderer internals, or direct `WindowRuntime` internals.
If a public example needs such a name, the missing author-facing facade belongs
in a focused public module first.
