#include "ui_internal.hpp"

namespace cgpui {

AppOpenedWindow WindowRuntime::open_window(WindowOptions options) {
  const WindowRuntimeId runtime_id = allocate_window_runtime_id();
  const WindowDescriptor descriptor = options.to_descriptor();
  AppOpenedWindow opened{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = {}};
  app_opened_windows_.push_back(opened);
  window_runtime_records_.push_back(WindowRuntimeRecord{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = {},
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = true,
      .owns_root_view = false,
      .active = false});
  activate_native_window_for_record(window_runtime_records_.back());
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
      .descriptor = descriptor,
      .root_view_id = root_view_id};
  app_opened_windows_.push_back(opened);
  window_runtime_records_.push_back(WindowRuntimeRecord{
      .runtime_id = runtime_id,
      .descriptor = descriptor,
      .root_view_id = root_view_id,
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = true,
      .owns_root_view = root_view_id.value != 0,
      .active = false});
  activate_native_window_for_record(window_runtime_records_.back());
  return opened;
}

std::span<const AppOpenedWindow> WindowRuntime::app_opened_windows() const {
  return app_opened_windows_;
}

} // namespace cgpui
