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
