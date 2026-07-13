#pragma once

#include "cgpui/app/window.hpp"
#include "cgpui/core/error.hpp"
#include "cgpui/core/events.hpp"
#include "cgpui/ui/runtime_ids.hpp"
#include "cgpui/ui/view.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>

namespace cgpui {

namespace detail {
struct TestAppState;
}

class TestAppWindow {
 public:
  [[nodiscard]] WindowRuntimeId runtime_id() const;
  [[nodiscard]] ViewId root_view_id() const;
  [[nodiscard]] WindowDescriptor descriptor() const;
  [[nodiscard]] Size viewport_size() const;
  [[nodiscard]] DpiScale scale() const;
  [[nodiscard]] bool active() const;
  [[nodiscard]] Window window() const;
  [[nodiscard]] View* root_view();
  [[nodiscard]] const View* root_view() const;
  [[nodiscard]] ViewInputState input_state() const;
  void dispatch_keystroke(KeyboardKey key) const;
  [[nodiscard]] bool simulate_keystrokes(std::string_view keystrokes) const;
  void dispatch_pointer_move(Point position) const;
  void dispatch_pointer_button(
      MouseButton button,
      bool pressed,
      Point position) const;
  void dispatch_pointer_scroll(Point delta, Point position) const;
  void dispatch_window_activation(bool active) const;
  void dispatch_window_focus(bool focused) const;
  void focus(ElementId element_id) const;
  void release_focus(ElementId element_id) const;

  template <typename T>
  [[nodiscard]] T* root_view_as() {
    return dynamic_cast<T*>(root_view());
  }

  template <typename T>
  [[nodiscard]] const T* root_view_as() const {
    return dynamic_cast<const T*>(root_view());
  }

 private:
  friend class TestApp;

  TestAppWindow(
      std::shared_ptr<detail::TestAppState> state,
      WindowRuntimeId runtime_id);

  std::shared_ptr<detail::TestAppState> state_;
  WindowRuntimeId runtime_id_{};
};

class TestApp {
 public:
  TestApp();
  TestApp(TestApp&&) noexcept;
  TestApp& operator=(TestApp&&) noexcept;
  TestApp(const TestApp&) = delete;
  TestApp& operator=(const TestApp&) = delete;
  ~TestApp();

  [[nodiscard]] TestAppWindow open_window(
      WindowOptions options,
      std::unique_ptr<View> root_view);
  [[nodiscard]] Result<TestAppWindow> try_open_window(
      WindowOptions options,
      std::unique_ptr<View> root_view);
  [[nodiscard]] std::size_t window_count() const;
  [[nodiscard]] std::optional<TestAppWindow> window(
      WindowRuntimeId runtime_id) const;
  void advance_time(std::uint64_t delta_ms) const;
  void run_until_parked() const;
  void advance_time_until_parked(std::uint64_t delta_ms) const;
  [[nodiscard]] bool cancel_timer(TimerId id) const;
  [[nodiscard]] bool complete_task(TaskId id) const;
  void drain_task_completions() const;

 private:
  std::shared_ptr<detail::TestAppState> state_;
};

} // namespace cgpui
