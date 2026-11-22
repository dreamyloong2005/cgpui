#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::RegisteredView {
  View* view = nullptr;
  std::unique_ptr<View> owned_view;
};

} // namespace cgpui
