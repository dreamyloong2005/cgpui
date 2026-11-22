#pragma once

#include "cgpui/ui/window_runtime.hpp"
#include "runtime_state_views_types_internal.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeViewState {
  ViewId root_view_id_{1};
  std::uint64_t next_view_id_ = 2;
  std::unordered_map<std::uint64_t, RegisteredView> view_registry_;
  std::unordered_set<std::uint64_t> removed_view_ids_;
};

} // namespace cgpui
