#include "cgpui/platform/platform_window.hpp"

#include <optional>
#include <string_view>

namespace {

class FakeWindow final : public cgpui::PlatformWindow {
 public:
  [[nodiscard]] cgpui::NativeSurfaceHandle native_surface() const override {
    return cgpui::Win32SurfaceHandle{};
  }

  [[nodiscard]] cgpui::WindowState state() const override {
    return state_;
  }

  void request_redraw() override {}

  void request_close() override {
    state_.close_requested = true;
  }

  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}

  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

 private:
  cgpui::WindowState state_{};
};

} // namespace

int main() {
  FakeWindow window;
  const cgpui::PlatformWindowLifecycleState initial = window.lifecycle_state();
  if (initial.native_window_created || initial.initial_configure_complete ||
      initial.active || initial.focused || initial.close_requested ||
      initial.display_state != cgpui::PlatformWindowDisplayState::normal) {
    return 1;
  }

  window.request_close();
  const cgpui::PlatformWindowLifecycleState closing = window.lifecycle_state();
  if (!closing.close_requested) {
    return 2;
  }
  return 0;
}
