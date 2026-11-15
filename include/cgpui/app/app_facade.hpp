#pragma once

#include "cgpui/app/window.hpp"
#include "cgpui/platform/platform_dpi_awareness.hpp"

#include <memory>
#include <optional>

namespace cgpui {

class App {
 public:
  explicit App(WindowRuntime& runtime);

  [[nodiscard]] WindowRuntime& runtime() const;
  [[nodiscard]] AppOpenedWindow open_window(WindowOptions options) const;
  [[nodiscard]] AppOpenedWindow open_window(
      WindowOptions options,
      std::unique_ptr<View> root_view) const;
  [[nodiscard]] Result<AppOpenedWindow> try_open_window(
      WindowOptions options) const;
  [[nodiscard]] Result<AppOpenedWindow> try_open_window(
      WindowOptions options,
      std::unique_ptr<View> root_view) const;
  [[nodiscard]] Window root_window() const;
  [[nodiscard]] std::optional<Window> window(
      WindowRuntimeId runtime_id) const;
  [[nodiscard]] SystemAppearance system_appearance() const;
  void set_system_appearance_callback(SystemAppearanceCallback callback) const;
  [[nodiscard]] PlatformDpiAwarenessState dpi_awareness() const;

 private:
  WindowRuntime* runtime_ = nullptr;
};

} // namespace cgpui
