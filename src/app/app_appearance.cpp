#include "cgpui/app/app_facade.hpp"

#include "cgpui/ui/window_runtime.hpp"

#include <utility>

namespace cgpui {

SystemAppearance App::system_appearance() const {
  return runtime_->system_appearance();
}

void App::set_system_appearance_callback(
    SystemAppearanceCallback callback) const {
  runtime_->set_system_appearance_callback(std::move(callback));
}

PlatformDpiAwarenessState App::dpi_awareness() const {
  return runtime_->dpi_awareness();
}

} // namespace cgpui
