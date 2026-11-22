#include "ui_internal.hpp"
#include "runtime_window_store_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

WindowRuntime::WindowRuntime(
    PlatformApplication& application,
    View& view,
    RendererFactory renderer_factory)
    : state_(std::make_unique<RuntimeState>(*this, application, view, std::move(renderer_factory))) {
  state_->views.view_registry_.insert_or_assign(
      state_->views.root_view_id_.value,
      RegisteredView{.view = &state_->core.view_});
  state_->windows.window_runtime_records_->insert(WindowRuntimeRecord{
      .runtime_id = state_->windows.root_window_runtime_id_,
      .parent_runtime_id = {},
      .descriptor = {},
      .root_view_id = state_->views.root_view_id_,
      .window = nullptr,
      .renderer = nullptr,
      .owns_window = true,
      .owns_renderer = false,
      .owns_root_view = false,
      .active = false});
}

WindowRuntime::WindowRuntime(PlatformApplication& application, View& view,
                             AppRendererFactory renderer_factory, OwnedRendererFactoryTag)
    : WindowRuntime(application, view, RendererFactory{}) {
  state_->windows.window_lifetime_->factory = std::move(renderer_factory);
  if (state_->windows.window_lifetime_->factory) {
    state_->core.renderer_factory_ = [this](const auto& descriptor) { return state_->windows.window_lifetime_->create(descriptor); };
  }
}

} // namespace cgpui
