#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

void WindowRuntime::install_static_tree_for_record(
    WindowRuntimeRecord& record, StaticElementTreeView tree) {
  auto& host = state_->elements.window_element_hosts_[record.runtime_id.value];
  state_->windows.window_lifetime_->retire_tree(std::move(host.tree));
  host.view_rendered = false;
  // Like the root static tree, storage remains borrowed from the owning View.
  host.static_tree = tree;
}

const StaticElementTreeView* WindowRuntime::static_tree_for_record(
    const WindowRuntimeRecord& record) const {
  const auto found = state_->elements.window_element_hosts_.find(record.runtime_id.value);
  if (found == state_->elements.window_element_hosts_.end()) return nullptr;
  const auto& tree = found->second.static_tree;
  return !tree.empty() && tree.valid() ? &tree : nullptr;
}

} // namespace cgpui
