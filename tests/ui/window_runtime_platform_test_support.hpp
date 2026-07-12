#pragma once

class FakeApplication final : public cgpui::PlatformApplication {
 public:
  explicit FakeApplication(FakeWindow& window) : window_(window) {}

  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    create_window_count += 1;
    last_descriptor = descriptor;
    window_.callback = std::move(callback);
    return std::unique_ptr<cgpui::PlatformWindow>(new BorrowedWindow(window_));
  }

  int run() override {
    run_count += 1;
    if (on_run) on_run();
    return run_result;
  }

  void quit() override { quit_count += 1; }
  void request_wakeup() override {
    request_wakeup_count += 1;
    wakeup_pending = true;
  }
  std::uint64_t monotonic_time_ms() const override {
    return monotonic_time_ms_value;
  }
  void request_wakeup_after(std::uint64_t delay_ms) override {
    delayed_wakeup_delays.push_back(delay_ms);
    delayed_wakeup_pending = true;
  }
  void cancel_wakeup_after() override {
    cancel_delayed_wakeup_count += 1;
    delayed_wakeup_pending = false;
  }
  void advance_monotonic_time(std::uint64_t delta_ms) {
    monotonic_time_ms_value += delta_ms;
  }
  void dispatch_delayed_wakeup() {
    if (!delayed_wakeup_pending) return;
    delayed_wakeup_pending = false;
    if (window_.callback) window_.callback(cgpui::WindowWakeupRequested{});
  }
  void dispatch_wakeup() {
    if (!wakeup_pending) return;
    wakeup_pending = false;
    if (window_.callback) window_.callback(cgpui::WindowWakeupRequested{});
  }

  FakeWindow& window_;
  int create_window_count = 0;
  int run_count = 0;
  int quit_count = 0;
  int run_result = 0;
  cgpui::WindowDescriptor last_descriptor{};
  void (*on_run)() = nullptr;
  int request_wakeup_count = 0;
  bool wakeup_pending = false;
  std::uint64_t monotonic_time_ms_value = 0;
  std::vector<std::uint64_t> delayed_wakeup_delays;
  std::size_t cancel_delayed_wakeup_count = 0;
  bool delayed_wakeup_pending = false;

 private:
  class BorrowedWindow final : public cgpui::PlatformWindow {
   public:
    explicit BorrowedWindow(FakeWindow& window) : window_(window) {}

    cgpui::NativeSurfaceHandle native_surface() const override {
      return window_.native_surface();
    }
    cgpui::WindowState state() const override { return window_.state(); }
    void request_redraw() override { window_.request_redraw(); }
    void request_close() override { window_.request_close(); }
    void set_title(std::string_view title) override { window_.set_title(title); }
    void set_cursor(cgpui::CursorShape shape) override {
      window_.set_cursor(shape);
    }
    void set_ime_text_input_placement(
        std::optional<cgpui::ImeTextInputPlacement> placement) override {
      window_.set_ime_text_input_placement(placement);
    }
    void update_accessibility_tree(
        cgpui::PlatformAccessibilityTreeUpdate update) override {
      window_.update_accessibility_tree(std::move(update));
    }

   private:
    FakeWindow& window_;
  };
};
