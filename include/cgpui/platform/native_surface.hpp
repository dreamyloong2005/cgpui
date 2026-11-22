#pragma once

#include <cstdint>
#include <variant>

namespace cgpui {

struct Win32SurfaceHandle {
  void* hinstance = nullptr;
  void* hwnd = nullptr;
  // The window has no redirection surface (popup windows): renderers
  // present through DirectComposition instead of a swapchain on it.
  bool composition = false;
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
