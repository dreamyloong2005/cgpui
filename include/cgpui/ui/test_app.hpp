#pragma once

#include "cgpui/app/window.hpp"
#include "cgpui/core/error.hpp"
#include "cgpui/ui/view.hpp"

#include <cstddef>
#include <memory>
#include <optional>

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

 private:
  std::shared_ptr<detail::TestAppState> state_;
};

} // namespace cgpui
