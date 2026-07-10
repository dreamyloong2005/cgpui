#include "ui_internal.hpp"

#include <expected>

namespace cgpui {

Result<AppOpenedWindow> WindowRuntime::try_open_window(
    WindowOptions options) {
  const WindowRuntimeId runtime_id = allocate_window_runtime_id();
  const WindowDescriptor descriptor = options.to_descriptor();
  AppOpenedWindow opened{
      .runtime_id = runtime_id,
      .parent_runtime_id = root_window_runtime_id_,
      .descriptor = descriptor,
      .root_view_id = {}};
  WindowRuntimeRecord record{
      .runtime_id = runtime_id,
      .parent_runtime_id = root_window_runtime_id_,
      .descriptor = descriptor,
      .root_view_id = {},
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = true,
      .owns_root_view = false,
      .active = false};
  activate_native_window_for_record(record);
  if (record.native_window_error.has_value()) {
    return std::unexpected(*record.native_window_error);
  }
  app_opened_windows_.push_back(opened);
  window_runtime_records_.push_back(record);
  return opened;
}

Result<AppOpenedWindow> WindowRuntime::try_open_window(
    WindowOptions options,
    std::unique_ptr<View> root_view) {
  const WindowRuntimeId runtime_id = allocate_window_runtime_id();
  const WindowDescriptor descriptor = options.to_descriptor();
  const ViewId root_view_id = register_view(std::move(root_view));
  AppOpenedWindow opened{
      .runtime_id = runtime_id,
      .parent_runtime_id = root_window_runtime_id_,
      .descriptor = descriptor,
      .root_view_id = root_view_id};
  WindowRuntimeRecord record{
      .runtime_id = runtime_id,
      .parent_runtime_id = root_window_runtime_id_,
      .descriptor = descriptor,
      .root_view_id = root_view_id,
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = true,
      .owns_root_view = root_view_id.value != 0,
      .active = false};
  activate_native_window_for_record(record);
  if (record.native_window_error.has_value()) {
    if (root_view_id.value != 0) {
      const bool removed_root_view = remove_view(root_view_id);
      (void)removed_root_view;
    }
    return std::unexpected(*record.native_window_error);
  }
  app_opened_windows_.push_back(opened);
  window_runtime_records_.push_back(record);
  return opened;
}

} // namespace cgpui
