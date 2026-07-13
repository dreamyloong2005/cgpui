#include "test_app_internal.hpp"

#include <stdexcept>

namespace cgpui::detail {

void TestAppState::request_redraw(WindowRuntimeId runtime_id) {
  WindowRuntimeRecord* record = runtime.find_window_runtime_record(runtime_id);
  if (record != nullptr && record->active && record->window != nullptr) {
    record->window->request_redraw();
  }
}

Result<void> TestAppState::try_draw_frame(WindowRuntimeId runtime_id) {
  WindowRuntimeRecord* record = runtime.find_window_runtime_record(runtime_id);
  View* view = record == nullptr ? nullptr : runtime.find_view(record->root_view_id);
  if (record == nullptr || !record->active || record->renderer == nullptr ||
      view == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "TestAppWindow cannot draw an inactive window"});
  }
  return runtime.try_draw_frame_for_record(*record, *view);
}

TestWindowRenderSnapshot TestAppState::render_snapshot(
    WindowRuntimeId runtime_id) const {
  const WindowRuntimeRecord* record = runtime.window_runtime_record(runtime_id);
  const auto* renderer = record == nullptr
      ? nullptr
      : dynamic_cast<const TestRenderer*>(record->renderer);
  return renderer == nullptr ? TestWindowRenderSnapshot{} : renderer->snapshot();
}

} // namespace cgpui::detail

namespace cgpui {

void TestAppWindow::simulate_resize(
    Size framebuffer_size,
    DpiScale scale) const {
  (void)state_->dispatch_event(
      runtime_id_, PlatformEvent{WindowResized{framebuffer_size, scale}});
}

void TestAppWindow::request_redraw() const { state_->request_redraw(runtime_id_); }

Result<void> TestAppWindow::try_draw_frame() const {
  return state_->try_draw_frame(runtime_id_);
}

void TestAppWindow::draw_frame() const {
  auto result = try_draw_frame();
  if (!result) throw std::runtime_error(result.error().message);
}

TestWindowRenderSnapshot TestAppWindow::render_snapshot() const {
  return state_->render_snapshot(runtime_id_);
}

} // namespace cgpui
