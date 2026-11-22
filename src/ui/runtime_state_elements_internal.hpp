#pragma once

#include "cgpui/ui/window_runtime.hpp"
#include "cgpui/ui/element_tree.hpp"
#include "runtime_state_elements_types_internal.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeElementState {
  std::unique_ptr<ElementTree> owned_element_tree_;
  bool view_rendered_element_tree_ = false;
  const Element* element_root_ = nullptr;
  StaticElementTreeView static_element_tree_;
  bool has_static_element_tree_ = false;
  std::unordered_map<std::uint64_t, CursorShape> element_cursors_;
  #include "runtime_window_element_host_internal.hpp"
};

} // namespace cgpui
