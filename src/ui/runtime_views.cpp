#include "ui_internal.hpp"

namespace cgpui {

View* WindowRuntime::root_view() {
  return find_view(root_view_id_);
}

const View* WindowRuntime::root_view() const {
  return find_view(root_view_id_);
}

ViewId WindowRuntime::register_view(View& view) {
  const ViewId view_id = allocate_view_id();
  removed_view_ids_.erase(view_id.value);
  view_registry_.insert_or_assign(
      view_id.value,
      RegisteredView{.view = &view});
  return view_id;
}

ViewId WindowRuntime::register_view(std::unique_ptr<View> view) {
  const ViewId view_id = allocate_view_id();
  removed_view_ids_.erase(view_id.value);
  if (view != nullptr) {
    View* view_ptr = view.get();
    view_registry_.insert_or_assign(
        view_id.value,
        RegisteredView{.view = view_ptr, .owned_view = std::move(view)});
  }
  return view_id;
}

View* WindowRuntime::find_view(ViewId view_id) {
  const auto entry = view_registry_.find(view_id.value);
  if (entry == view_registry_.end()) {
    return nullptr;
  }
  return entry->second.view;
}

const View* WindowRuntime::find_view(ViewId view_id) const {
  const auto entry = view_registry_.find(view_id.value);
  if (entry == view_registry_.end()) {
    return nullptr;
  }
  return entry->second.view;
}

bool WindowRuntime::remove_view(ViewId view_id) {
  if (view_id.value == 0 || view_id == root_view_id_) {
    return false;
  }
  const auto entry = view_registry_.find(view_id.value);
  if (entry == view_registry_.end()) {
    return false;
  }
  view_registry_.erase(entry);
  removed_view_ids_.insert(view_id.value);
  remove_subscriptions_for_view(view_id);
  remove_observers_for_view(view_id);
  return true;
}

ViewId WindowRuntime::allocate_view_id() {
  const ViewId view_id{next_view_id_};
  next_view_id_ += 1;
  return view_id;
}

bool WindowRuntime::is_view_id_allocated(ViewId view_id) const {
  return view_id.value != 0 && view_id.value < next_view_id_ &&
         !removed_view_ids_.contains(view_id.value);
}

std::optional<ViewId> WindowRuntime::upgrade_view(WeakView view) const {
  const auto context_token = reinterpret_cast<std::uintptr_t>(this);
  if (view.empty() || !view.matches_context(context_token) ||
      !is_view_id_allocated(view.id())) {
    return std::nullopt;
  }
  return view.id();
}

} // namespace cgpui
