#include "ui_internal.hpp"
#include "runtime_window_store_internal.hpp"

namespace cgpui {

AppOpenedWindow WindowRuntime::open_window(WindowOptions options) {
  const WindowRuntimeId runtime_id = allocate_window_runtime_id();
  const WindowDescriptor descriptor = options.to_descriptor();
  AppOpenedWindow opened{
      .runtime_id = runtime_id,
      .parent_runtime_id = state_->windows.root_window_runtime_id_,
      .descriptor = descriptor,
      .root_view_id = {}};
  state_->windows.app_opened_windows_.push_back(opened);
  auto& record = state_->windows.window_runtime_records_->insert(WindowRuntimeRecord{
      .runtime_id = runtime_id,
      .parent_runtime_id = state_->windows.root_window_runtime_id_,
      .descriptor = descriptor,
      .root_view_id = {},
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = false,
      .owns_root_view = false,
      .active = false});
  activate_native_window_for_record(record);
  return opened;
}

AppOpenedWindow WindowRuntime::open_window(
    WindowOptions options,
    std::unique_ptr<View> root_view) {
  const WindowRuntimeId runtime_id = allocate_window_runtime_id();
  const WindowDescriptor descriptor = options.to_descriptor();
  const ViewId root_view_id = register_view(std::move(root_view));
  AppOpenedWindow opened{
      .runtime_id = runtime_id,
      .parent_runtime_id = state_->windows.root_window_runtime_id_,
      .descriptor = descriptor,
      .root_view_id = root_view_id};
  state_->windows.app_opened_windows_.push_back(opened);
  auto& record = state_->windows.window_runtime_records_->insert(WindowRuntimeRecord{
      .runtime_id = runtime_id,
      .parent_runtime_id = state_->windows.root_window_runtime_id_,
      .descriptor = descriptor,
      .root_view_id = root_view_id,
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = false,
      .owns_root_view = root_view_id.value != 0,
      .active = false});
  activate_native_window_for_record(record);
  return opened;
}

std::span<const AppOpenedWindow> WindowRuntime::app_opened_windows() const {
  return state_->windows.app_opened_windows_;
}

} // namespace cgpui
