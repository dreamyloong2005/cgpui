#include "ui_internal.hpp"
#include "runtime_async_io_internal.hpp"
#include "runtime_task_pool_internal.hpp"
#include "runtime_task_group_internal.hpp"

namespace cgpui {

WindowRuntime::WindowRuntime(
    PlatformApplication& application,
    View& view,
    RendererFactory renderer_factory)
    : application_(application),
      view_(view),
      renderer_factory_(std::move(renderer_factory)),
      async_io_registry_(std::make_unique<RuntimeAsyncIoRegistry>()),
      task_pool_(std::make_unique<RuntimeTaskPool>()),
      task_group_store_(std::make_unique<RuntimeTaskGroupStore>()) {
  view_registry_.insert_or_assign(
      root_view_id_.value,
      RegisteredView{.view = &view_});
  window_runtime_records_.push_back(WindowRuntimeRecord{
      .runtime_id = root_window_runtime_id_,
      .parent_runtime_id = {},
      .descriptor = {},
      .root_view_id = root_view_id_,
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = false,
      .owns_root_view = false,
      .active = false});
}

} // namespace cgpui
