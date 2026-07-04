#include "cgpui/app/app_facade.hpp"

#include "cgpui/ui/window_runtime.hpp"

#include <utility>

namespace cgpui {

App::App(WindowRuntime& runtime) : runtime_(&runtime) {}

WindowRuntime& App::runtime() const {
  return *runtime_;
}

AppOpenedWindow App::open_window(WindowOptions options) const {
  return runtime_->open_window(std::move(options));
}

AppOpenedWindow App::open_window(
    WindowOptions options,
    std::unique_ptr<View> root_view) const {
  return runtime_->open_window(std::move(options), std::move(root_view));
}

Window App::root_window() const {
  return Window(*runtime_, runtime_->root_window_runtime_id());
}

std::optional<Window> App::window(WindowRuntimeId runtime_id) const {
  if (runtime_->window_runtime_record(runtime_id) == nullptr) {
    return std::nullopt;
  }
  return Window(*runtime_, runtime_id);
}

} // namespace cgpui
