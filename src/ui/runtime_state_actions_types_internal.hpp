#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::ActionDispatchTarget {
  std::optional<ElementId> focus_owner;
  std::optional<ViewId> view_id;
  std::function<WindowRuntimeContext()> context;
};

class WindowRuntime::ActionWindowScope {
 public:
  ActionWindowScope(std::optional<WindowRuntimeId>& slot, WindowRuntimeId id)
      : slot_(slot), outer_(std::exchange(slot, id)) {}
  ActionWindowScope(const ActionWindowScope&) = delete;
  ActionWindowScope& operator=(const ActionWindowScope&) = delete;
  ~ActionWindowScope() { slot_ = outer_; }

 private:
  std::optional<WindowRuntimeId>& slot_;
  std::optional<WindowRuntimeId> outer_;
};

} // namespace cgpui
