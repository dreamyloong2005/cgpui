#include "ui_internal.hpp"
#include "runtime_frame_retry_internal.hpp"

namespace cgpui {

void WindowRuntime::handle_redraw_for_record(
    WindowRuntimeRecord& record,
    View& view) {
  auto result = try_draw_frame_for_record(record, view);
  if (!result && !runtime_retry_frame(*this, record.runtime_id, result.error())) {
    fail_and_quit(result.error());
  }
}

} // namespace cgpui
