# Architecture

## Module Ownership

- `include/cgpui/core` and `src/core`: errors, geometry, events, entities,
  assets, font/paint value types, and pixel-scale conversions.
- `include/cgpui/app` and `src/app`: application and window facades.
- `include/cgpui/platform` and `src/platform`: platform-neutral contracts,
  font discovery/database, and focused Win32, Wayland, X11, and Cocoa implementations.
- `include/cgpui/renderer` and `src/renderer`: renderer contracts, neutral text
  shaping/measurement/rasterization under `text/`, and focused Vulkan/Metal backends.
- `include/cgpui/ui` and `src/ui`: views, elements, layout, editable text models,
  IME/interaction, scheduling, widgets, testing, and runtime orchestration.

Platform and renderer do not include UI headers. The old `ui/text_*.hpp`
entrypoints for shared text primitives remain thin compatibility includes;
their implementation and native-private headers belong to `renderer/text`.

Aggregate headers remain thin. Public non-template bodies live in focused
`.cpp` files; template and constexpr bodies stay in focused public leaves.

[Native text](native-text.md) documents native shaping (DirectWrite on
Windows, HarfBuzz + FreeType + fontconfig on Linux), font fallback, metrics,
the shape cache, glyph-id rasterization into the Vulkan atlas, and support limits.

## Public Authoring Boundary

`cgpui/prelude.hpp` is the preferred application include. `App`, `Window`,
`Context<T>`, `ViewContext`, builders, elements, widgets, tasks, and Result
types keep application code outside runtime, renderer, and platform internals.

## Static Fast Path

Static element traversal uses compact records and spans. A View can opt into
`render_static(...)`, and the runtime routes the installed static tree without
requiring polymorphic allocation for every node.

## Dynamic Escape Hatch

`AnyElement`, `ElementTree`, runtime-owned entities, callback wrappers, and
dynamic View rendering remain available where runtime variability is required.
The Dynamic escape hatch is explicit so static paths do not silently acquire
heap allocation, broad type erasure, or avoidable virtual dispatch.

## Runtime Flow

1. A platform application creates a native window and surface handle.
2. The renderer backend binds the surface and owns frame resources.
3. `WindowRuntime` routes platform events into the root View and element tree.
4. Layout and paint produce platform-neutral renderer commands. Each frame
   the runtime calls `View::render`, lays out the returned element tree in
   window coordinates, then `render_view` paints `View::paint` first and the
   element tree on top of it, in the root and every additional window.
   `View::paint` is the view's own canvas beneath its elements. Static
   element trees (`render_static`) hold structure, bounds and text without
   paint style; they drive hit testing and accessibility, and such views
   draw in `View::paint`.
5. Tasks, timers, animations, and cross-thread entity work wake the owning
   runtime and drain on its thread.

[Runtime lifecycle](runtime-lifecycle.md) defines callback, window-resource,
retry and popup capability boundaries. [Renderer planning](renderer-planning.md)
describes shared CPU plans that do not require a GPU backend dependency.
[Runtime state isolation](runtime-state-isolation.md) describes the installed
header boundary, private state groups and terminal shutdown ordering.

## Linux Backend Selection

Linux compiles focused Wayland and X11 targets. Selection is deterministic:
`CGPUI_LINUX_BACKEND`, then `WAYLAND_DISPLAY`, then `DISPLAY`. Vulkan enables
only the surface extension required by the selected native handle.

## macOS Verification Boundary

Cocoa and Metal share the public runtime and renderer contracts. Native CI,
performance and stress reports, and Release SDKs target only arm64, with a
macOS 26.0 deployment target. The historical Phase J verification records
both architectures under xmake. For recorded mcpp evidence and
revalidation requirements see [macOS validation](macos-validation.md).

## Structure Enforcement

Architecture tests guard leaf ownership, aggregate-header cleanliness, source
line budgets, mcpp manifest registrations, and public/private include boundaries. A
behavior change is incomplete if these structure tests regress.

Structure tests assert current ownership and registration contracts, not exact
completion sentences, handoff labels, or entries in working logs. Historical
plans and verification records remain evidence; changing their prose must not
break otherwise unchanged implementation tests. Documentation tests may still
check current public concepts, schema fields, and valid navigation.
