#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::StaticRenderInstallResult {
  bool installed = false;
  ElementId root_id;
  std::size_t node_count = 0;
};

struct WindowRuntime::WindowElementHost {
  std::unique_ptr<ElementTree> tree;
  bool view_rendered = false;
  std::unordered_map<std::uint64_t, CursorShape> cursors;
};

} // namespace cgpui
