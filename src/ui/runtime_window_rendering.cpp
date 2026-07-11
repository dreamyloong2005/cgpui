#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::handle_redraw_for_record(
    WindowRuntimeRecord& record,
    View& view) {
  auto result = try_draw_frame_for_record(record, view);
  record.redraw_scheduled = false;
  if (!result) {
    fail_and_quit(result.error());
  }
}

} // namespace cgpui
