#include "runtime_frame_retry_internal.hpp"

#include "cgpui/core/error.hpp"
#include "cgpui/app/window.hpp"

namespace cgpui {

bool runtime_retry_frame(
    WindowRuntime& runtime, WindowRuntimeId window, const Error& error) {
  if (error.code != ErrorCode::frame_retry_required) return false;
  Window(runtime, window).request_render();
  return true;
}

} // namespace cgpui
