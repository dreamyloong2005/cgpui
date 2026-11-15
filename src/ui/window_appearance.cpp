#include "cgpui/app/window.hpp"

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

// System appearance is process-wide on every supported backend, so each
// window reports the application snapshot.
SystemAppearance Window::appearance() const {
  return runtime_->system_appearance();
}

} // namespace cgpui
