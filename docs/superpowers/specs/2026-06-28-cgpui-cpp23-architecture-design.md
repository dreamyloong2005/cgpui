# CGPUI C++23 Architecture Design

## Purpose

CGPUI is a C++23-native UI framework inspired by GPUI's core ideas, not a direct Rust API translation. The first stage establishes cross-platform architecture boundaries before building a full widget system or polishing public APIs.

The initial milestone is `examples/hello_window`: create a native desktop window on Windows, macOS, and Linux, attach the correct graphics surface, clear the frame, and draw one rectangle through a unified framework-facing interface.

## Scope

The first stage targets desktop platforms only:

- Windows: Win32 windowing with Vulkan rendering.
- macOS: Cocoa/Objective-C++ windowing with Metal rendering.
- Linux: platform abstraction accounts for Wayland and X11, with Wayland implemented first and Vulkan rendering.

The build system is Xmake with C++23 enabled for all C++ targets.

## Zero-Cost Abstraction Principle

CGPUI should use C++23 abstractions only when they compile down to direct,
predictable runtime work on hot paths. The framework can expose pleasant
declarative APIs, but layout, reconciliation, event routing, render command
generation, reactive invalidation, and renderer submission must not
accidentally add heap allocation, broad type erasure, virtual dispatch, or
extra handle translation compared with a direct C++ implementation.

Preferred patterns:

- use templates, concepts, `constexpr`, `std::expected`, and `std::optional`
  for type safety and compile-time selection;
- keep static widget composition and dynamic widget/plugin escape hatches as
  separate paths;
- lower builder APIs into retained node storage, compact value records, spans,
  or command buffers before hot traversal;
- measure or test abstraction boundaries when they could affect allocation,
  dispatch, or per-frame work.

Dynamic dispatch and type erasure are allowed only where the behavior is truly
dynamic, such as plugin/editor surfaces, runtime-generated widgets, or platform
backend boundaries. They should be visible in the owning module and should not
become the default implementation mechanism for static UI composition.

Initial enforcement after Phase C:

- `ElementId`, `ViewId`, and `ElementKey` live in a small public id leaf so
  static UI data can reference ids without including the dynamic `Element`
  hierarchy.
- `StaticElementTreeView` is the first compact fast path: it consumes dense
  `StaticElementNode` records and `std::span<const ElementId>` child lists for
  traversal and hit testing without virtual dispatch, `std::function`,
  `AnyElement`, `std::any`, or `dynamic_cast`.
- `StaticRender` is separate from the existing `Render` concept. The existing
  `IntoElement = AnyElement` path remains the explicit dynamic escape hatch
  for runtime-generated or plugin/editor UI.

## Architecture

### Core

`cgpui_core` owns portable foundation types:

- application lifecycle primitives
- error and result types
- geometry, DPI, and coordinate types
- window descriptors
- input and window event types
- time and task scheduling interfaces

`cgpui_core` must not include Win32, Cocoa, Wayland, X11, Vulkan, or Metal headers.

### Platform Abstraction

`cgpui_platform` defines the platform-facing interfaces consumed by the app/runtime layer:

- `PlatformApplication`
- `PlatformWindow`
- `PlatformEventLoop`
- `NativeSurfaceHandle`
- clipboard, cursor, drag/drop, and IME interface slots

The first milestone requires real window creation, event dispatch, resize handling, DPI reporting, close handling, pointer movement, pointer buttons, and keyboard key events. Clipboard, drag/drop, and IME can be interface-only placeholders in this stage.

### Platform Implementations

`cgpui_platform_win32` implements Win32 windows, the Win32 message loop, input translation, DPI handling, and Vulkan-compatible native surface handles.

`cgpui_platform_macos` implements Cocoa windows in Objective-C++, AppKit event integration, backing scale reporting, and CAMetalLayer-backed native surface handles.

`cgpui_platform_linux_wayland` implements Wayland display connection, surface creation, shell integration, event dispatch, pointer/keyboard translation, output scaling, and Vulkan-compatible native surface handles.

`cgpui_platform_linux_x11` is not implemented in the first milestone, but the Linux platform interface must not assume Wayland-only concepts.

### Renderer Abstraction

`cgpui_renderer` defines backend-independent rendering concepts:

- render device
- render surface
- swapchain or presentable frame
- command encoder
- texture and buffer handles
- pipeline handles
- render pass
- paint primitives

The public renderer abstraction should expose only the operations needed by the first milestone: begin frame, clear surface, draw a solid rectangle, end frame, and present.

### Renderer Implementations

`cgpui_renderer_vulkan` implements the renderer abstraction for Windows and Linux. Vulkan code must be shared between Win32 and Wayland wherever possible; platform-specific behavior belongs in surface creation and adapter capability glue, not duplicated renderer code.

`cgpui_renderer_metal` implements the renderer abstraction for macOS. Metal code is compiled only for macOS targets.

### UI Runtime

`cgpui_ui` is intentionally small in the first stage. It provides:

- a minimal view entrypoint
- a simple layout rectangle
- a paint command list
- invalidation hooks sufficient to redraw the window

The first milestone supports only background fill and solid rectangle paint commands. Text rendering, accessibility, animation, complex layout, focus traversal, and widget composition are out of scope.

## Dependency Direction

Dependencies must flow inward and stay explicit:

- examples depend on `cgpui_ui`, `cgpui_platform_*`, and the selected renderer backend.
- `cgpui_ui` depends on `cgpui_core` and `cgpui_renderer`.
- platform implementations depend on `cgpui_platform` and `cgpui_core`.
- renderer implementations depend on `cgpui_renderer`, `cgpui_core`, and the relevant graphics API.
- `cgpui_core`, `cgpui_platform`, and `cgpui_renderer` remain platform/API-header clean.

The demo must be written against framework abstractions, not directly against Win32, Cocoa, Wayland, Vulkan, or Metal.

## Xmake Layout

The first implementation should create separate targets that reflect architecture boundaries rather than a single monolithic library:

- `cgpui_core`
- `cgpui_platform`
- `cgpui_platform_win32`
- `cgpui_platform_macos`
- `cgpui_platform_linux_wayland`
- `cgpui_renderer`
- `cgpui_renderer_vulkan`
- `cgpui_renderer_metal`
- `cgpui_ui`
- `hello_window`

Targets that depend on platform SDKs or graphics APIs should be enabled only on compatible hosts.

## First Milestone Acceptance Criteria

`examples/hello_window` is accepted when:

- Windows creates a Win32 window, initializes Vulkan, clears the surface, and draws one solid rectangle.
- Linux creates a Wayland window, initializes Vulkan, clears the surface, and draws one solid rectangle.
- macOS creates a Cocoa window, initializes Metal, clears the surface, and draws one solid rectangle.
- resizing updates the render surface on each implemented platform.
- closing the window exits cleanly.
- core headers compile without platform or graphics API headers.
- the public demo code uses only CGPUI abstractions.

## Deferred Work

The first stage intentionally defers:

- full GPUI API compatibility
- full widget library
- text shaping and font fallback
- IME implementation
- clipboard implementation
- drag and drop implementation
- native menus
- accessibility
- animations
- retained view diffing
- advanced layout
- X11 implementation

These features should be introduced only after the platform and renderer boundaries have been validated by the first milestone.
