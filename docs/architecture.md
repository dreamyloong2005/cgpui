# Architecture

## Module Ownership

- `include/cgpui/core` and `src/core`: errors, geometry, events, entities,
  assets, and platform-neutral value types.
- `include/cgpui/app` and `src/app`: application and window facades.
- `include/cgpui/platform` and `src/platform`: platform-neutral contracts plus
  focused Win32, Wayland, X11, and Cocoa implementations.
- `include/cgpui/renderer` and `src/renderer`: renderer contracts plus focused
  Vulkan and Metal implementations.
- `include/cgpui/ui` and `src/ui`: views, elements, layout, text, scheduling,
  widgets, testing, and runtime orchestration.

Aggregate headers remain thin. Public non-template bodies live in focused
`.cpp` files; template and constexpr bodies stay in focused public leaves.

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
4. Layout and paint produce platform-neutral renderer commands.
5. Tasks, timers, animations, and cross-thread entity work wake the owning
   runtime and drain on its thread.

## Linux Backend Selection

Linux compiles focused Wayland and X11 targets. Selection is deterministic:
`CGPUI_LINUX_BACKEND`, then `WAYLAND_DISPLAY`, then `DISPLAY`. Vulkan enables
only the surface extension required by the selected native handle.

## macOS Verification Boundary

Cocoa and Metal share the public runtime and renderer contracts while native
evidence remains architecture-specific. Phase J retains separate arm64 and
x86_64 reports and Release artifacts at deployment target 13.0. Both native
CI matrices pass and are recorded in the final verification authority.

## Structure Enforcement

Architecture tests guard leaf ownership, aggregate-header cleanliness, source
line budgets, Xmake target modules, and public/private include boundaries. A
behavior change is incomplete if these structure tests regress.
