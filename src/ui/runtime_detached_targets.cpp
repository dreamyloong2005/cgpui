#include "runtime_detached_targets_internal.hpp"

namespace cgpui {
namespace {

class DetachedWindow final : public PlatformWindow {
 public:
  [[nodiscard]] NativeSurfaceHandle native_surface() const override {
    return {};
  }
  [[nodiscard]] WindowState state() const override { return {}; }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<ImeTextInputPlacement>) override {}
};

class DetachedRenderer final : public Renderer {
 public:
  Result<void> resize(Size, DpiScale) override { return unavailable(); }
  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    return std::unexpected(unavailable().error());
  }

 private:
  static Result<void> unavailable() {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "the window runtime has no window"});
  }
};

} // namespace

PlatformWindow& detached_runtime_window() {
  static DetachedWindow window;
  return window;
}

Renderer& detached_runtime_renderer() {
  static DetachedRenderer renderer;
  return renderer;
}

} // namespace cgpui
