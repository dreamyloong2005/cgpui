#include "cgpui/app/window.hpp"
#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

RendererBackendInfo Window::backend_info() const {
  const auto* runtime_record = record();
  return runtime_record && runtime_record->active && runtime_record->renderer
      ? runtime_record->renderer->backend_info() : RendererBackendInfo{};
}

} // namespace cgpui
