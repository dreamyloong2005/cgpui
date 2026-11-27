#pragma once

#include "runtime_window_lifecycle_test_support.hpp"

namespace runtime_window_cursor_test {

struct CursorState {
  std::shared_ptr<runtime_window_test::NativeState> native;
  std::function<void(cgpui::CursorShape)> on_cursor;
  cgpui::CursorShape cursor = cgpui::CursorShape::default_arrow;
};

class CursorWindow final : public cgpui::PlatformWindow {
 public:
  CursorWindow(std::unique_ptr<cgpui::PlatformWindow> delegate,
               std::shared_ptr<CursorState> state)
      : delegate_(std::move(delegate)), state_(std::move(state)) {
    state_->native->native_address = this;
  }
  cgpui::NativeSurfaceHandle native_surface() const override { return delegate_->native_surface(); }
  cgpui::WindowState state() const override { return delegate_->state(); }
  void request_redraw() override { delegate_->request_redraw(); }
  void request_close() override { delegate_->request_close(); }
  void set_title(std::string_view title) override { delegate_->set_title(title); }
  void set_cursor(cgpui::CursorShape cursor) override {
    state_->cursor = cursor;
    const auto callback = state_->on_cursor;
    if (callback) callback(cursor);
  }
  void set_ime_text_input_placement(std::optional<cgpui::ImeTextInputPlacement> placement) override {
    delegate_->set_ime_text_input_placement(placement);
  }
  cgpui::PlatformPopupPlacementResult place_popup(cgpui::WindowPopupPlacement placement) override {
    return delegate_->place_popup(placement);
  }

 private:
  std::unique_ptr<cgpui::PlatformWindow> delegate_;
  std::shared_ptr<CursorState> state_;
};

class CursorApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor, cgpui::PlatformEventCallback callback) override {
    auto window = base.create_window(descriptor, std::move(callback));
    if (!window) return std::unexpected(window.error());
    root = std::make_shared<CursorState>();
    root->native = base.last_created;
    return std::make_unique<CursorWindow>(std::move(*window), root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor, cgpui::PlatformWindow& parent,
      cgpui::PlatformEventCallback callback) override {
    auto window = base.create_child_window(descriptor, parent, std::move(callback));
    if (!window) return std::unexpected(window.error());
    auto state = std::make_shared<CursorState>();
    state->native = base.last_created;
    children.push_back(state);
    return std::make_unique<CursorWindow>(std::move(*window), std::move(state));
  }
  bool popup_windows_supported() const override { return true; }
  std::uint64_t monotonic_time_ms() const override { return base.monotonic_time_ms(); }
  int run() override { return base.run(); }
  void quit() override { base.quit(); }
  void request_wakeup() override { base.request_wakeup(); }

  runtime_window_test::Application base;
  std::shared_ptr<CursorState> root;
  std::vector<std::shared_ptr<CursorState>> children;
};

} // namespace runtime_window_cursor_test
