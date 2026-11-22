#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::TextPointerSelectionDrag {
  ElementId element_id;
  std::size_t anchor_offset = 0;
};

} // namespace cgpui
