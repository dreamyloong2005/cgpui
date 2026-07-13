#include "test_app_internal.hpp"

#include <utility>

namespace cgpui::detail {

TestPlatformWindow::TestPlatformWindow(
    WindowDescriptor descriptor,
    PlatformEventCallback callback)
    : descriptor_(std::move(descriptor)), callback_(std::move(callback)) {}

NativeSurfaceHandle TestPlatformWindow::native_surface() const {
  return Win32SurfaceHandle{};
}

WindowState TestPlatformWindow::state() const {
  return WindowState{
      .framebuffer_size = descriptor_.size,
      .scale = DpiScale{1.0F},
      .close_requested = false};
}

void TestPlatformWindow::request_redraw() {
  if (callback_) callback_(WindowRedrawRequested{});
}

void TestPlatformWindow::request_close() {
  if (callback_) callback_(WindowCloseRequested{});
}

void TestPlatformWindow::set_title(std::string_view title) {
  descriptor_.title = title;
}

void TestPlatformWindow::set_cursor(CursorShape) {}

void TestPlatformWindow::set_ime_text_input_placement(
    std::optional<ImeTextInputPlacement>) {}

void TestPlatformWindow::dispatch_event(const PlatformEvent& event) {
  if (callback_) callback_(event);
}

Result<std::unique_ptr<PlatformWindow>>
TestPlatformApplication::create_window(
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback) {
  return std::make_unique<TestPlatformWindow>(descriptor, std::move(callback));
}

Result<std::unique_ptr<PlatformWindow>>
TestPlatformApplication::create_child_window(
    const WindowDescriptor& descriptor,
    PlatformWindow&,
    PlatformEventCallback callback) {
  return create_window(descriptor, std::move(callback));
}

std::uint64_t TestPlatformApplication::monotonic_time_ms() const { return 0; }

int TestPlatformApplication::run() { return 0; }

void TestPlatformApplication::quit() {}

} // namespace cgpui::detail
