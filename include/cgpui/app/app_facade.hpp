#pragma once

#include "cgpui/app/window.hpp"

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

 private:
  WindowRuntime* runtime_ = nullptr;
};

} // namespace cgpui
