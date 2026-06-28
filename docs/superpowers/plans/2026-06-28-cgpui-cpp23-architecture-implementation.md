# CGPUI C++23 Architecture Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first executable CGPUI C++23 architecture foundation: clean public interfaces, Xmake targets, minimal UI paint flow, and platform/renderer backend boundaries.

**Architecture:** The implementation starts with platform/API-clean core interfaces, then adds platform-specific targets and renderer-specific targets behind those interfaces. Windows and Linux share the Vulkan renderer boundary, macOS uses the Metal renderer boundary, and the demo depends only on CGPUI abstractions.

**Tech Stack:** C++23, Xmake, Win32, Wayland, Cocoa/Objective-C++, Vulkan, Metal.

---

## Scope

This is the first implementation plan for the approved architecture spec. It creates the foundation that later plans will extend into full native Wayland shell integration and real Vulkan/Metal rectangle rendering.

This plan is complete when:

- public core/platform/renderer/ui headers build without platform or graphics API headers.
- Xmake has separate targets for each architecture boundary.
- `examples/hello_window` is written only against CGPUI abstractions.
- Windows has a real Win32 platform target.
- Linux has a Wayland connection target and surface-handle type.
- macOS has Cocoa and Metal target boundaries.
- Windows/Linux have a Vulkan renderer target boundary.
- macOS has a Metal renderer target boundary.

## File Structure

- `xmake.lua`: root project configuration, C++23 policy, target graph, platform-specific target gating.
- `include/cgpui/core/geometry.hpp`: portable geometry, DPI, color, and rectangle types.
- `include/cgpui/core/error.hpp`: portable error/result vocabulary.
- `include/cgpui/core/events.hpp`: portable input and window event types.
- `include/cgpui/core/window.hpp`: portable window descriptors and window state types.
- `include/cgpui/platform/native_surface.hpp`: type-erased native surface handles.
- `include/cgpui/platform/platform.hpp`: `PlatformApplication`, `PlatformWindow`, and event interfaces.
- `include/cgpui/renderer/renderer.hpp`: backend-independent renderer interfaces.
- `include/cgpui/ui/ui.hpp`: minimal view, paint command, and frame driver interfaces.
- `src/core/empty.cpp`: anchor translation unit for `cgpui_core`.
- `src/platform/empty.cpp`: anchor translation unit for `cgpui_platform`.
- `src/platform/platform_unimplemented.cpp`: fallback platform factory for unsupported hosts.
- `src/platform/win32/win32_application.cpp`: Win32 platform implementation.
- `src/platform/linux/wayland_application.cpp`: Wayland connection implementation.
- `src/platform/macos/macos_application.mm`: Cocoa platform implementation.
- `src/renderer/empty.cpp`: anchor translation unit for `cgpui_renderer`.
- `src/renderer/renderer_unimplemented.cpp`: fallback renderer factory for unsupported hosts.
- `src/renderer/vulkan/vulkan_renderer.cpp`: Vulkan renderer boundary for Windows/Linux.
- `src/renderer/metal/metal_renderer.mm`: Metal renderer boundary for macOS.
- `src/ui/ui.cpp`: minimal UI paint command implementation.
- `examples/hello_window/main.cpp`: unified demo entrypoint.
- `tests/header_cleanliness/core_header_cleanliness.cpp`: public core/platform/renderer header build test.
- `tests/header_cleanliness/ui_header_cleanliness.cpp`: public UI header build test.

## Task 1: Scaffold Xmake Targets And Public Core Headers

**Files:**
- Create: `xmake.lua`
- Create: `include/cgpui/core/geometry.hpp`
- Create: `include/cgpui/core/error.hpp`
- Create: `include/cgpui/core/events.hpp`
- Create: `include/cgpui/core/window.hpp`
- Create: `src/core/empty.cpp`
- Create: `tests/header_cleanliness/core_header_cleanliness.cpp`

- [ ] **Step 1: Create the root Xmake project**

Write `xmake.lua`:

```lua
set_project("cgpui")
set_version("0.1.0")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

local public_includedirs = "include"

target("cgpui_core")
    set_kind("static")
    add_files("src/core/*.cpp")
    add_includedirs(public_includedirs, {public = true})

target("core_header_cleanliness")
    set_kind("binary")
    add_files("tests/header_cleanliness/core_header_cleanliness.cpp")
    add_deps("cgpui_core")
    add_includedirs(public_includedirs)
```

- [ ] **Step 2: Add geometry types**

Write `include/cgpui/core/geometry.hpp`:

```cpp
#pragma once

namespace cgpui {

struct Size {
  float width = 0.0F;
  float height = 0.0F;
};

struct Point {
  float x = 0.0F;
  float y = 0.0F;
};

struct Rect {
  Point origin;
  Size size;
};

struct DpiScale {
  float value = 1.0F;
};

struct Color {
  float r = 0.0F;
  float g = 0.0F;
  float b = 0.0F;
  float a = 1.0F;
};

} // namespace cgpui
```

- [ ] **Step 3: Add error vocabulary**

Write `include/cgpui/core/error.hpp`:

```cpp
#pragma once

#include <expected>
#include <string>

namespace cgpui {

enum class ErrorCode {
  platform_initialization_failed,
  window_creation_failed,
  renderer_initialization_failed,
  frame_acquisition_failed,
  unsupported_platform,
};

struct Error {
  ErrorCode code;
  std::string message;
};

template <typename T>
using Result = std::expected<T, Error>;

} // namespace cgpui
```

- [ ] **Step 4: Add event types**

Write `include/cgpui/core/events.hpp`:

```cpp
#pragma once

#include "cgpui/core/geometry.hpp"

#include <cstdint>
#include <variant>

namespace cgpui {

enum class MouseButton {
  left,
  right,
  middle,
  other,
};

enum class KeyAction {
  pressed,
  released,
};

struct WindowCloseRequested {};

struct WindowResized {
  Size size;
  DpiScale scale;
};

struct PointerMoved {
  Point position;
};

struct PointerButton {
  MouseButton button;
  bool pressed = false;
  Point position;
};

struct KeyboardKey {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
};

using PlatformEvent = std::variant<
    WindowCloseRequested,
    WindowResized,
    PointerMoved,
    PointerButton,
    KeyboardKey>;

} // namespace cgpui
```

- [ ] **Step 5: Add window descriptors**

Write `include/cgpui/core/window.hpp`:

```cpp
#pragma once

#include "cgpui/core/geometry.hpp"

#include <string>

namespace cgpui {

struct WindowDescriptor {
  std::string title = "CGPUI";
  Size size{1280.0F, 720.0F};
};

struct WindowState {
  Size framebuffer_size;
  DpiScale scale;
  bool close_requested = false;
};

} // namespace cgpui
```

- [ ] **Step 6: Add translation unit and header test**

Write `src/core/empty.cpp`:

```cpp
#include "cgpui/core/window.hpp"

namespace cgpui {
void cgpui_core_anchor() {}
} // namespace cgpui
```

Write `tests/header_cleanliness/core_header_cleanliness.cpp`:

```cpp
#include "cgpui/core/error.hpp"
#include "cgpui/core/events.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/core/window.hpp"

int main() {
  cgpui::WindowDescriptor descriptor;
  cgpui::WindowState state;
  state.framebuffer_size = descriptor.size;
  return state.framebuffer_size.width > 0.0F ? 0 : 1;
}
```

- [ ] **Step 7: Build and run the header test**

Run:

```bash
xmake f -c -m debug
xmake build core_header_cleanliness
xmake run core_header_cleanliness
```

Expected: configure succeeds, build succeeds, executable exits with code `0`.

- [ ] **Step 8: Commit**

```bash
git add xmake.lua include/cgpui/core src/core tests/header_cleanliness
git commit -m "feat: scaffold cgpui core"
```

## Task 2: Add Platform And Renderer Abstraction Interfaces

**Files:**
- Modify: `xmake.lua`
- Create: `include/cgpui/platform/native_surface.hpp`
- Create: `include/cgpui/platform/platform.hpp`
- Create: `include/cgpui/renderer/renderer.hpp`
- Create: `src/platform/empty.cpp`
- Create: `src/renderer/empty.cpp`
- Modify: `tests/header_cleanliness/core_header_cleanliness.cpp`

- [ ] **Step 1: Extend Xmake targets**

Replace `xmake.lua` with:

```lua
set_project("cgpui")
set_version("0.1.0")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

local public_includedirs = "include"

target("cgpui_core")
    set_kind("static")
    add_files("src/core/*.cpp")
    add_includedirs(public_includedirs, {public = true})

target("cgpui_platform")
    set_kind("static")
    add_files("src/platform/*.cpp")
    add_deps("cgpui_core")
    add_includedirs(public_includedirs, {public = true})

target("cgpui_renderer")
    set_kind("static")
    add_files("src/renderer/*.cpp")
    add_deps("cgpui_core")
    add_includedirs(public_includedirs, {public = true})

target("core_header_cleanliness")
    set_kind("binary")
    add_files("tests/header_cleanliness/core_header_cleanliness.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
    add_includedirs(public_includedirs)
```

- [ ] **Step 2: Add native surface handle types**

Write `include/cgpui/platform/native_surface.hpp`:

```cpp
#pragma once

#include <cstdint>
#include <variant>

namespace cgpui {

struct Win32SurfaceHandle {
  void* hinstance = nullptr;
  void* hwnd = nullptr;
};

struct WaylandSurfaceHandle {
  void* display = nullptr;
  void* surface = nullptr;
};

struct X11SurfaceHandle {
  void* display = nullptr;
  std::uint64_t window = 0;
};

struct MetalSurfaceHandle {
  void* layer = nullptr;
};

using NativeSurfaceHandle = std::variant<
    Win32SurfaceHandle,
    WaylandSurfaceHandle,
    X11SurfaceHandle,
    MetalSurfaceHandle>;

} // namespace cgpui
```

- [ ] **Step 3: Add platform interfaces**

Write `include/cgpui/platform/platform.hpp`:

```cpp
#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/core/events.hpp"
#include "cgpui/core/window.hpp"
#include "cgpui/platform/native_surface.hpp"

#include <functional>
#include <memory>
#include <string_view>

namespace cgpui {

class PlatformWindow {
 public:
  virtual ~PlatformWindow() = default;

  [[nodiscard]] virtual NativeSurfaceHandle native_surface() const = 0;
  [[nodiscard]] virtual WindowState state() const = 0;
  virtual void request_redraw() = 0;
  virtual void set_title(std::string_view title) = 0;
};

using PlatformEventCallback = std::function<void(const PlatformEvent&)>;

class PlatformApplication {
 public:
  virtual ~PlatformApplication() = default;

  virtual Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) = 0;

  virtual int run() = 0;
  virtual void quit() = 0;
};

Result<std::unique_ptr<PlatformApplication>> create_platform_application();

} // namespace cgpui
```

- [ ] **Step 4: Add renderer interfaces**

Write `include/cgpui/renderer/renderer.hpp`:

```cpp
#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/platform/native_surface.hpp"

#include <memory>

namespace cgpui {

struct RenderSurfaceDescriptor {
  NativeSurfaceHandle native_surface;
  Size framebuffer_size;
  DpiScale scale;
};

struct SolidRect {
  Rect rect;
  Color color;
};

class RenderFrame {
 public:
  virtual ~RenderFrame() = default;
  virtual void clear(Color color) = 0;
  virtual void draw_rect(const SolidRect& rect) = 0;
  virtual Result<void> present() = 0;
};

class Renderer {
 public:
  virtual ~Renderer() = default;

  virtual Result<void> resize(Size framebuffer_size, DpiScale scale) = 0;
  virtual Result<std::unique_ptr<RenderFrame>> begin_frame() = 0;
};

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor);

} // namespace cgpui
```

- [ ] **Step 5: Add anchor translation units**

Write `src/platform/empty.cpp`:

```cpp
#include "cgpui/platform/platform.hpp"

namespace cgpui {
void cgpui_platform_anchor() {}
} // namespace cgpui
```

Write `src/renderer/empty.cpp`:

```cpp
#include "cgpui/renderer/renderer.hpp"

namespace cgpui {
void cgpui_renderer_anchor() {}
} // namespace cgpui
```

- [ ] **Step 6: Expand header test**

Write `tests/header_cleanliness/core_header_cleanliness.cpp`:

```cpp
#include "cgpui/core/error.hpp"
#include "cgpui/core/events.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/core/window.hpp"
#include "cgpui/platform/native_surface.hpp"
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

int main() {
  cgpui::WindowDescriptor descriptor;
  cgpui::WindowState state;
  state.framebuffer_size = descriptor.size;

  cgpui::Win32SurfaceHandle win32_surface;
  cgpui::NativeSurfaceHandle surface = win32_surface;
  (void)surface;

  return state.framebuffer_size.width > 0.0F ? 0 : 1;
}
```

- [ ] **Step 7: Build and run the header test**

Run:

```bash
xmake build core_header_cleanliness
xmake run core_header_cleanliness
```

Expected: build succeeds and executable exits with code `0`.

- [ ] **Step 8: Commit**

```bash
git add xmake.lua include/cgpui/platform include/cgpui/renderer src/platform src/renderer tests/header_cleanliness/core_header_cleanliness.cpp
git commit -m "feat: define platform and renderer interfaces"
```

## Task 3: Add Minimal UI Runtime And Paint Commands

**Files:**
- Modify: `xmake.lua`
- Create: `include/cgpui/ui/ui.hpp`
- Create: `src/ui/ui.cpp`
- Create: `tests/header_cleanliness/ui_header_cleanliness.cpp`

- [ ] **Step 1: Add `cgpui_ui` and UI header test targets**

Append these targets to `xmake.lua`:

```lua
target("cgpui_ui")
    set_kind("static")
    add_files("src/ui/*.cpp")
    add_deps("cgpui_core", "cgpui_renderer")
    add_includedirs(public_includedirs, {public = true})

target("ui_header_cleanliness")
    set_kind("binary")
    add_files("tests/header_cleanliness/ui_header_cleanliness.cpp")
    add_deps("cgpui_core", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
```

- [ ] **Step 2: Add UI interfaces**

Write `include/cgpui/ui/ui.hpp`:

```cpp
#pragma once

#include "cgpui/core/geometry.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <span>
#include <vector>

namespace cgpui {

struct PaintCommand {
  SolidRect solid_rect;
};

class PaintList {
 public:
  void clear();
  void fill_rect(Rect rect, Color color);
  [[nodiscard]] std::span<const PaintCommand> commands() const;

 private:
  std::vector<PaintCommand> commands_;
};

class View {
 public:
  virtual ~View() = default;
  virtual void paint(PaintList& paint_list, Size viewport_size) = 0;
};

Result<void> render_view(Renderer& renderer, View& view, Size viewport_size);

} // namespace cgpui
```

- [ ] **Step 3: Implement UI paint flow**

Write `src/ui/ui.cpp`:

```cpp
#include "cgpui/ui/ui.hpp"

#include <expected>

namespace cgpui {

void PaintList::clear() {
  commands_.clear();
}

void PaintList::fill_rect(Rect rect, Color color) {
  commands_.push_back(PaintCommand{.solid_rect = SolidRect{.rect = rect, .color = color}});
}

std::span<const PaintCommand> PaintList::commands() const {
  return commands_;
}

Result<void> render_view(Renderer& renderer, View& view, Size viewport_size) {
  auto frame = renderer.begin_frame();
  if (!frame) {
    return std::unexpected(frame.error());
  }

  (*frame)->clear(Color{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F});

  PaintList paint_list;
  view.paint(paint_list, viewport_size);
  for (const auto& command : paint_list.commands()) {
    (*frame)->draw_rect(command.solid_rect);
  }

  return (*frame)->present();
}

} // namespace cgpui
```

- [ ] **Step 4: Add UI header test**

Write `tests/header_cleanliness/ui_header_cleanliness.cpp`:

```cpp
#include "cgpui/ui/ui.hpp"

class TestView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.fill_rect(
        cgpui::Rect{.origin = {10.0F, 10.0F}, .size = {20.0F, 20.0F}},
        cgpui::Color{.r = 1.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F});
  }
};

int main() {
  cgpui::PaintList paint_list;
  TestView view;
  view.paint(paint_list, cgpui::Size{100.0F, 100.0F});
  return paint_list.commands().size() == 1 ? 0 : 1;
}
```

- [ ] **Step 5: Build and run header tests**

Run:

```bash
xmake build core_header_cleanliness
xmake build ui_header_cleanliness
xmake run core_header_cleanliness
xmake run ui_header_cleanliness
```

Expected: both tests build and exit with code `0`.

- [ ] **Step 6: Commit**

```bash
git add xmake.lua include/cgpui/ui src/ui tests/header_cleanliness/ui_header_cleanliness.cpp
git commit -m "feat: add minimal ui paint runtime"
```

## Task 4: Add Portable Factories And Hello Window Demo

**Files:**
- Modify: `xmake.lua`
- Create: `src/platform/platform_unimplemented.cpp`
- Create: `src/renderer/renderer_unimplemented.cpp`
- Create: `examples/hello_window/main.cpp`

- [ ] **Step 1: Add portable factory fallbacks**

Write `src/platform/platform_unimplemented.cpp`:

```cpp
#include "cgpui/platform/platform.hpp"

#include <expected>

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__linux__)

namespace cgpui {

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::unexpected(Error{
      .code = ErrorCode::unsupported_platform,
      .message = "No platform implementation is enabled for this host"});
}

} // namespace cgpui

#endif
```

Write `src/renderer/renderer_unimplemented.cpp`:

```cpp
#include "cgpui/renderer/renderer.hpp"

#include <expected>

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__linux__)

namespace cgpui {

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor&) {
  return std::unexpected(Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = "No renderer implementation is enabled for this host"});
}

} // namespace cgpui

#endif
```

- [ ] **Step 2: Add hello window demo**

Write `examples/hello_window/main.cpp`:

```cpp
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "cgpui/ui/ui.hpp"

#include <iostream>
#include <memory>
#include <variant>

class HelloView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size viewport_size) override {
    const auto rect_size = cgpui::Size{viewport_size.width * 0.35F, viewport_size.height * 0.25F};
    paint_list.fill_rect(
        cgpui::Rect{
            .origin = {viewport_size.width * 0.325F, viewport_size.height * 0.375F},
            .size = rect_size},
        cgpui::Color{.r = 0.23F, .g = 0.55F, .b = 0.86F, .a = 1.0F});
  }
};

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  std::unique_ptr<cgpui::Renderer> renderer;
  HelloView view;
  cgpui::Size viewport_size{960.0F, 640.0F};
  bool should_quit = false;

  auto window_result = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Hello Window", .size = viewport_size},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          should_quit = true;
          (*app)->quit();
          return;
        }

        if (const auto* resized = std::get_if<cgpui::WindowResized>(&event); resized != nullptr) {
          viewport_size = resized->size;
          if (renderer) {
            auto resized_result = renderer->resize(resized->size, resized->scale);
            if (!resized_result) {
              std::cerr << resized_result.error().message << '\n';
            }
          }
        }

        if (renderer && !should_quit) {
          auto render_result = cgpui::render_view(*renderer, view, viewport_size);
          if (!render_result) {
            std::cerr << render_result.error().message << '\n';
          }
        }
      });

  if (!window_result) {
    std::cerr << window_result.error().message << '\n';
    return 1;
  }

  auto window = std::move(*window_result);
  viewport_size = window->state().framebuffer_size;

  auto renderer_result = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window->native_surface(),
      .framebuffer_size = window->state().framebuffer_size,
      .scale = window->state().scale});
  if (!renderer_result) {
    std::cerr << renderer_result.error().message << '\n';
    return 1;
  }

  renderer = std::move(*renderer_result);
  window->request_redraw();
  return (*app)->run();
}
```

- [ ] **Step 3: Add demo target**

Append this target to `xmake.lua`:

```lua
target("hello_window")
    set_kind("binary")
    add_files("examples/hello_window/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
```

- [ ] **Step 4: Build header tests and demo**

Run:

```bash
xmake build core_header_cleanliness
xmake build ui_header_cleanliness
xmake build hello_window
```

Expected: header tests build. `hello_window` may fail to link on Windows, macOS, or Linux until the matching platform and renderer targets are added in later tasks of this plan.

- [ ] **Step 5: Commit**

```bash
git add xmake.lua src/platform/platform_unimplemented.cpp src/renderer/renderer_unimplemented.cpp examples/hello_window
git commit -m "feat: add hello window demo entrypoint"
```

## Task 5: Add Win32 Platform Target

**Files:**
- Modify: `xmake.lua`
- Create: `src/platform/win32/win32_application.cpp`

- [ ] **Step 1: Add Win32 target to Xmake**

Append this block after the `hello_window` target in `xmake.lua`:

```lua
if is_plat("windows") then
    target("cgpui_platform_win32")
        set_kind("static")
        add_files("src/platform/win32/*.cpp")
        add_deps("cgpui_core", "cgpui_platform")
        add_includedirs(public_includedirs, {public = true})
        add_syslinks("user32", "gdi32", "shell32")

    target("hello_window")
        add_deps("cgpui_platform_win32")
end
```

- [ ] **Step 2: Implement Win32 application and window**

Write `src/platform/win32/win32_application.cpp`:

```cpp
#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace cgpui {
namespace {

std::wstring widen(std::string_view value) {
  if (value.empty()) {
    return {};
  }
  const auto required = MultiByteToWideChar(
      CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
  std::wstring result(static_cast<std::size_t>(required), L'\0');
  MultiByteToWideChar(
      CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), required);
  return result;
}

class Win32Window final : public PlatformWindow {
 public:
  Win32Window(HINSTANCE instance, PlatformEventCallback callback, WindowState state)
      : instance_(instance), callback_(std::move(callback)), state_(state) {}

  void attach(HWND hwnd) {
    hwnd_ = hwnd;
  }

  NativeSurfaceHandle native_surface() const override {
    return Win32SurfaceHandle{.hinstance = instance_, .hwnd = hwnd_};
  }

  WindowState state() const override {
    return state_;
  }

  void request_redraw() override {
    InvalidateRect(hwnd_, nullptr, FALSE);
  }

  void set_title(std::string_view title) override {
    const auto wide_title = widen(title);
    SetWindowTextW(hwnd_, wide_title.c_str());
  }

  void update_size() {
    RECT rect{};
    GetClientRect(hwnd_, &rect);
    const auto width = static_cast<float>(rect.right - rect.left);
    const auto height = static_cast<float>(rect.bottom - rect.top);
    const auto dpi = static_cast<float>(GetDpiForWindow(hwnd_));
    state_.framebuffer_size = Size{width, height};
    state_.scale = DpiScale{dpi / 96.0F};
    callback_(WindowResized{.size = state_.framebuffer_size, .scale = state_.scale});
  }

  void close_requested() {
    state_.close_requested = true;
    callback_(WindowCloseRequested{});
  }

  void pointer_moved(LPARAM lparam) {
    callback_(PointerMoved{.position = Point{
        static_cast<float>(GET_X_LPARAM(lparam)),
        static_cast<float>(GET_Y_LPARAM(lparam))}});
  }

  void pointer_button(MouseButton button, bool pressed, LPARAM lparam) {
    callback_(PointerButton{
        .button = button,
        .pressed = pressed,
        .position = Point{
            static_cast<float>(GET_X_LPARAM(lparam)),
            static_cast<float>(GET_Y_LPARAM(lparam))}});
  }

  void key_event(WPARAM wparam, KeyAction action) {
    callback_(KeyboardKey{.key_code = static_cast<std::uint32_t>(wparam), .action = action});
  }

 private:
  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
  PlatformEventCallback callback_;
  WindowState state_;
};

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  auto* window = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  switch (message) {
    case WM_NCCREATE: {
      const auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
      auto* created_window = static_cast<Win32Window*>(create->lpCreateParams);
      created_window->attach(hwnd);
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(created_window));
      return TRUE;
    }
    case WM_SIZE:
      if (window != nullptr) {
        window->update_size();
      }
      return 0;
    case WM_CLOSE:
      if (window != nullptr) {
        window->close_requested();
      }
      return 0;
    case WM_MOUSEMOVE:
      if (window != nullptr) {
        window->pointer_moved(lparam);
      }
      return 0;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      if (window != nullptr) {
        window->pointer_button(MouseButton::left, message == WM_LBUTTONDOWN, lparam);
      }
      return 0;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      if (window != nullptr) {
        window->pointer_button(MouseButton::right, message == WM_RBUTTONDOWN, lparam);
      }
      return 0;
    case WM_KEYDOWN:
      if (window != nullptr) {
        window->key_event(wparam, KeyAction::pressed);
      }
      return 0;
    case WM_KEYUP:
      if (window != nullptr) {
        window->key_event(wparam, KeyAction::released);
      }
      return 0;
    default:
      return DefWindowProcW(hwnd, message, wparam, lparam);
  }
}

class Win32Application final : public PlatformApplication {
 public:
  Win32Application() : instance_(GetModuleHandleW(nullptr)) {}

  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override {
    const wchar_t* class_name = L"CGPUIWindow";

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance_;
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    window_class.lpszClassName = class_name;
    RegisterClassExW(&window_class);

    auto state = WindowState{
        .framebuffer_size = descriptor.size,
        .scale = DpiScale{1.0F},
        .close_requested = false};
    auto window = std::make_unique<Win32Window>(instance_, std::move(callback), state);

    const auto title = widen(descriptor.title);
    HWND hwnd = CreateWindowExW(
        0,
        class_name,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        static_cast<int>(descriptor.size.width),
        static_cast<int>(descriptor.size.height),
        nullptr,
        nullptr,
        instance_,
        window.get());
    if (hwnd == nullptr) {
      return std::unexpected(Error{
          .code = ErrorCode::window_creation_failed,
          .message = "CreateWindowExW failed"});
    }

    ShowWindow(hwnd, SW_SHOW);
    window->update_size();
    return window;
  }

  int run() override {
    MSG message{};
    while (running_ && GetMessageW(&message, nullptr, 0, 0) > 0) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    return 0;
  }

  void quit() override {
    running_ = false;
    PostQuitMessage(0);
  }

 private:
  HINSTANCE instance_ = nullptr;
  bool running_ = true;
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<Win32Application>();
}

} // namespace cgpui
```

- [ ] **Step 3: Build on Windows**

Run on Windows:

```bash
xmake f -c -m debug
xmake build hello_window
```

Expected: build still fails to link `create_renderer` until Task 6 adds the Vulkan renderer target, or succeeds on hosts where renderer fallback is enabled.

- [ ] **Step 4: Commit**

```bash
git add xmake.lua src/platform/win32
git commit -m "feat: add win32 platform boundary"
```

## Task 6: Add Vulkan Renderer Target Boundary

**Files:**
- Modify: `xmake.lua`
- Create: `src/renderer/vulkan/vulkan_renderer.cpp`

- [ ] **Step 1: Add Vulkan renderer target**

Append this block after the `hello_window` target in `xmake.lua`:

```lua
if is_plat("windows", "linux") then
    add_requires("vulkansdk")

    target("cgpui_renderer_vulkan")
        set_kind("static")
        add_files("src/renderer/vulkan/*.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
        add_packages("vulkansdk")
        add_includedirs(public_includedirs, {public = true})

    target("hello_window")
        add_deps("cgpui_renderer_vulkan")
end
```

- [ ] **Step 2: Implement Vulkan renderer boundary**

Write `src/renderer/vulkan/vulkan_renderer.cpp`:

```cpp
#include "cgpui/renderer/renderer.hpp"

#include <vulkan/vulkan.h>

#include <expected>
#include <memory>

namespace cgpui {
namespace {

class VulkanFrame final : public RenderFrame {
 public:
  void clear(Color color) override {
    clear_color_ = color;
  }

  void draw_rect(const SolidRect& rect) override {
    rect_ = rect;
  }

  Result<void> present() override {
    return {};
  }

 private:
  Color clear_color_{};
  SolidRect rect_{};
};

class VulkanRenderer final : public Renderer {
 public:
  explicit VulkanRenderer(RenderSurfaceDescriptor descriptor)
      : descriptor_(std::move(descriptor)) {}

  Result<void> resize(Size framebuffer_size, DpiScale scale) override {
    descriptor_.framebuffer_size = framebuffer_size;
    descriptor_.scale = scale;
    return {};
  }

  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    return std::make_unique<VulkanFrame>();
  }

 private:
  RenderSurfaceDescriptor descriptor_;
  VkInstance instance_ = VK_NULL_HANDLE;
};

} // namespace

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  if (descriptor.framebuffer_size.width <= 0.0F || descriptor.framebuffer_size.height <= 0.0F) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a non-empty framebuffer"});
  }

  return std::make_unique<VulkanRenderer>(descriptor);
}

} // namespace cgpui
```

- [ ] **Step 3: Build on Windows or Linux**

Run on Windows or Linux:

```bash
xmake f -c -m debug
xmake build hello_window
```

Expected: build succeeds when the Vulkan SDK package is available. Running the demo can open a native platform window once the current host has a platform target; rendering calls return successfully without drawing real GPU output in this foundation plan.

- [ ] **Step 4: Commit**

```bash
git add xmake.lua src/renderer/vulkan
git commit -m "feat: add vulkan renderer boundary"
```

## Task 7: Add Linux Wayland Target Boundary

**Files:**
- Modify: `xmake.lua`
- Create: `src/platform/linux/wayland_application.cpp`

- [ ] **Step 1: Add Wayland target gated to Linux**

Append this block after the `hello_window` target in `xmake.lua`:

```lua
if is_plat("linux") then
    add_requires("wayland")

    target("cgpui_platform_linux_wayland")
        set_kind("static")
        add_files("src/platform/linux/*.cpp")
        add_deps("cgpui_core", "cgpui_platform")
        add_packages("wayland")
        add_includedirs(public_includedirs, {public = true})

    target("hello_window")
        add_deps("cgpui_platform_linux_wayland")
end
```

- [ ] **Step 2: Implement Wayland connection boundary**

Write `src/platform/linux/wayland_application.cpp`:

```cpp
#include "cgpui/platform/platform.hpp"

#include <wayland-client.h>

#include <expected>
#include <memory>
#include <string_view>
#include <utility>

namespace cgpui {
namespace {

class WaylandWindow final : public PlatformWindow {
 public:
  WaylandWindow(wl_display* display, PlatformEventCallback callback, WindowState state)
      : display_(display), callback_(std::move(callback)), state_(state) {}

  NativeSurfaceHandle native_surface() const override {
    return WaylandSurfaceHandle{.display = display_, .surface = nullptr};
  }

  WindowState state() const override {
    return state_;
  }

  void request_redraw() override {
    callback_(WindowResized{.size = state_.framebuffer_size, .scale = state_.scale});
  }

  void set_title(std::string_view) override {}

 private:
  wl_display* display_ = nullptr;
  PlatformEventCallback callback_;
  WindowState state_;
};

class WaylandApplication final : public PlatformApplication {
 public:
  WaylandApplication() : display_(wl_display_connect(nullptr)) {}

  ~WaylandApplication() override {
    if (display_ != nullptr) {
      wl_display_disconnect(display_);
    }
  }

  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override {
    if (display_ == nullptr) {
      return std::unexpected(Error{
          .code = ErrorCode::platform_initialization_failed,
          .message = "wl_display_connect failed"});
    }

    auto state = WindowState{
        .framebuffer_size = descriptor.size,
        .scale = DpiScale{1.0F},
        .close_requested = false};
    return std::make_unique<WaylandWindow>(display_, std::move(callback), state);
  }

  int run() override {
    while (running_ && display_ != nullptr) {
      wl_display_dispatch_pending(display_);
      wl_display_flush(display_);
      running_ = false;
    }
    return 0;
  }

  void quit() override {
    running_ = false;
  }

 private:
  wl_display* display_ = nullptr;
  bool running_ = true;
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<WaylandApplication>();
}

} // namespace cgpui
```

- [ ] **Step 3: Build on Linux**

Run on Linux:

```bash
xmake f -c -m debug
xmake build hello_window
```

Expected: build succeeds when Wayland client development libraries and the Vulkan SDK package are available.

- [ ] **Step 4: Commit**

```bash
git add xmake.lua src/platform/linux
git commit -m "feat: add wayland platform boundary"
```

## Task 8: Add macOS Cocoa And Metal Target Boundaries

**Files:**
- Modify: `xmake.lua`
- Create: `src/platform/macos/macos_application.mm`
- Create: `src/renderer/metal/metal_renderer.mm`

- [ ] **Step 1: Add macOS targets**

Append this block after the `hello_window` target in `xmake.lua`:

```lua
if is_plat("macosx") then
    target("cgpui_platform_macos")
        set_kind("static")
        add_files("src/platform/macos/*.mm")
        add_deps("cgpui_core", "cgpui_platform")
        add_frameworks("AppKit", "QuartzCore")
        add_includedirs(public_includedirs, {public = true})

    target("cgpui_renderer_metal")
        set_kind("static")
        add_files("src/renderer/metal/*.mm")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
        add_frameworks("Metal", "QuartzCore")
        add_includedirs(public_includedirs, {public = true})

    target("hello_window")
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
end
```

- [ ] **Step 2: Implement Cocoa platform boundary**

Write `src/platform/macos/macos_application.mm`:

```objective-c++
#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include <expected>
#include <memory>
#include <string_view>
#include <utility>

namespace cgpui {
namespace {

class MacOSWindow final : public PlatformWindow {
 public:
  MacOSWindow(NSWindow* window, CAMetalLayer* layer, PlatformEventCallback callback, WindowState state)
      : window_(window), layer_(layer), callback_(std::move(callback)), state_(state) {}

  NativeSurfaceHandle native_surface() const override {
    return MetalSurfaceHandle{.layer = layer_};
  }

  WindowState state() const override {
    return state_;
  }

  void request_redraw() override {
    callback_(WindowResized{.size = state_.framebuffer_size, .scale = state_.scale});
  }

  void set_title(std::string_view title) override {
    NSString* value = [[NSString alloc] initWithBytes:title.data()
                                               length:title.size()
                                             encoding:NSUTF8StringEncoding];
    [window_ setTitle:value];
  }

 private:
  NSWindow* window_ = nil;
  CAMetalLayer* layer_ = nil;
  PlatformEventCallback callback_;
  WindowState state_;
};

class MacOSApplication final : public PlatformApplication {
 public:
  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    const auto rect = NSMakeRect(0, 0, descriptor.size.width, descriptor.size.height);
    NSWindow* window = [[NSWindow alloc]
        initWithContentRect:rect
                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                    backing:NSBackingStoreBuffered
                      defer:NO];
    if (window == nil) {
      return std::unexpected(Error{
          .code = ErrorCode::window_creation_failed,
          .message = "NSWindow creation failed"});
    }

    CAMetalLayer* layer = [CAMetalLayer layer];
    [[window contentView] setWantsLayer:YES];
    [[window contentView] setLayer:layer];
    [window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];

    auto state = WindowState{
        .framebuffer_size = descriptor.size,
        .scale = DpiScale{static_cast<float>([window backingScaleFactor])},
        .close_requested = false};
    return std::make_unique<MacOSWindow>(window, layer, std::move(callback), state);
  }

  int run() override {
    [NSApp run];
    return 0;
  }

  void quit() override {
    [NSApp terminate:nil];
  }
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<MacOSApplication>();
}

} // namespace cgpui
```

- [ ] **Step 3: Implement Metal renderer boundary**

Write `src/renderer/metal/metal_renderer.mm`:

```objective-c++
#include "cgpui/renderer/renderer.hpp"

#import <Metal/Metal.h>

#include <expected>
#include <memory>

namespace cgpui {
namespace {

class MetalFrame final : public RenderFrame {
 public:
  void clear(Color color) override {
    clear_color_ = color;
  }

  void draw_rect(const SolidRect& rect) override {
    rect_ = rect;
  }

  Result<void> present() override {
    return {};
  }

 private:
  Color clear_color_{};
  SolidRect rect_{};
};

class MetalRenderer final : public Renderer {
 public:
  explicit MetalRenderer(RenderSurfaceDescriptor descriptor)
      : descriptor_(std::move(descriptor)), device_(MTLCreateSystemDefaultDevice()) {}

  Result<void> resize(Size framebuffer_size, DpiScale scale) override {
    descriptor_.framebuffer_size = framebuffer_size;
    descriptor_.scale = scale;
    return {};
  }

  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    if (device_ == nil) {
      return std::unexpected(Error{
          .code = ErrorCode::renderer_initialization_failed,
          .message = "MTLCreateSystemDefaultDevice failed"});
    }
    return std::make_unique<MetalFrame>();
  }

 private:
  RenderSurfaceDescriptor descriptor_;
  id<MTLDevice> device_ = nil;
};

} // namespace

Result<std::unique_ptr<Renderer>> create_renderer(
    const RenderSurfaceDescriptor& descriptor) {
  return std::make_unique<MetalRenderer>(descriptor);
}

} // namespace cgpui
```

- [ ] **Step 4: Build on macOS**

Run on macOS:

```bash
xmake f -c -m debug
xmake build hello_window
```

Expected: `hello_window` builds and links AppKit, QuartzCore, and Metal.

- [ ] **Step 5: Commit**

```bash
git add xmake.lua src/platform/macos src/renderer/metal
git commit -m "feat: add macos metal boundaries"
```

## Task 9: Add README And Verify Foundation

**Files:**
- Create: `README.md`

- [ ] **Step 1: Add README build instructions**

Write `README.md`:

```markdown
# CGPUI

CGPUI is a C++23-native UI framework inspired by GPUI's architecture. The first milestone focuses on platform and renderer boundaries.

## Build

```bash
xmake f -c -m debug
xmake build hello_window
```

## Run

```bash
xmake run hello_window
```

## Platform Targets

- Windows: Win32 + Vulkan
- Linux: Wayland + Vulkan, with X11 reserved in the platform abstraction
- macOS: Cocoa + Metal

## Current Foundation

The current foundation defines architecture boundaries and compiles backend targets. Native surface completion and real GPU draw submission are planned as separate implementation passes.
```

- [ ] **Step 2: Verify public headers stay clean**

Run:

```bash
xmake build core_header_cleanliness
xmake build ui_header_cleanliness
xmake run core_header_cleanliness
xmake run ui_header_cleanliness
```

Expected: all commands succeed.

- [ ] **Step 3: Verify target graph**

Run:

```bash
xmake show -t hello_window
```

Expected: `hello_window` depends on the base `cgpui_*` targets and only the platform/renderer targets for the current host.

- [ ] **Step 4: Build demo**

Run:

```bash
xmake build hello_window
```

Expected: build succeeds on a host with the required platform SDKs and renderer package available.

- [ ] **Step 5: Commit**

```bash
git add README.md
git commit -m "docs: document architecture foundation build"
```

## Plan Self-Review

- Spec coverage in this plan: core types, platform abstraction, renderer abstraction, minimal UI runtime, Xmake layout, demo abstraction boundary, Win32 target, Wayland target boundary, Vulkan target boundary, macOS Cocoa target boundary, Metal target boundary, and header cleanliness.
- Spec coverage in later implementation passes: complete Wayland xdg-shell surface creation, real Vulkan clear/rectangle rendering, real Metal clear/rectangle rendering, resize-driven swapchain/layer updates, and visual acceptance for `hello_window`.
- Red-flag scan: no incomplete requirement markers remain in this plan.
- Type consistency: public names used across tasks are `PlatformApplication`, `PlatformWindow`, `NativeSurfaceHandle`, `Renderer`, `RenderFrame`, `PaintList`, `View`, and `render_view`.
