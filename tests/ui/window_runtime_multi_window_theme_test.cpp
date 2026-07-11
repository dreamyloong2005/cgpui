#include "cgpui/cgpui.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace {

struct NativeWindow {
  cgpui::WindowState state;
  cgpui::PlatformEventCallback callback;
  int redraw_count = 0;
};

class ThemeWindow final : public cgpui::PlatformWindow {
 public:
  explicit ThemeWindow(NativeWindow& window) : window_(window) {}
  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return window_.state; }
  void request_redraw() override { ++window_.redraw_count; }
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}

 private:
  NativeWindow& window_;
};

class ThemeApplication final : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformEventCallback callback) override {
    root.state = {.framebuffer_size = descriptor.size,
                  .scale = cgpui::DpiScale{1.0F}};
    root.callback = std::move(callback);
    return std::make_unique<ThemeWindow>(root);
  }
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_child_window(
      const cgpui::WindowDescriptor& descriptor,
      cgpui::PlatformWindow&,
      cgpui::PlatformEventCallback callback) override {
    child.state = {.framebuffer_size = descriptor.size,
                   .scale = cgpui::DpiScale{1.0F}};
    child.callback = std::move(callback);
    return std::make_unique<ThemeWindow>(child);
  }
  int run() override {
    on_run();
    return 0;
  }
  void quit() override {}

  NativeWindow root;
  NativeWindow child;
  std::function<void()> on_run;
};

class ThemeFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class ThemeRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override {
    return {};
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::make_unique<ThemeFrame>();
  }
};

class ThemeView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}
};

bool color_matches(std::optional<cgpui::Color> color, cgpui::Color expected) {
  return color.has_value() && color->r == expected.r && color->g == expected.g &&
      color->b == expected.b && color->a == expected.a;
}

} // namespace

int main() {
  ThemeApplication application;
  ThemeView root_view;
  std::vector<std::unique_ptr<ThemeRenderer>> renderers;
  cgpui::WindowRuntime runtime(
      application,
      root_view,
      [&](const cgpui::RenderSurfaceDescriptor&)
          -> cgpui::Result<cgpui::Renderer*> {
        auto renderer = std::make_unique<ThemeRenderer>();
        cgpui::Renderer* raw = renderer.get();
        renderers.push_back(std::move(renderer));
        return raw;
      });
  const cgpui::AppOpenedWindow opened = runtime.open_window(
      cgpui::WindowOptions{}.title("Theme Child").size(320.0F, 200.0F),
      std::make_unique<ThemeView>());
  const cgpui::ThemeTokenId accent = cgpui::theme_token("accent");
  const cgpui::ThemeTokenId gap = cgpui::theme_token("gap");
  const cgpui::Color app_color = cgpui::rgb(10, 20, 30);
  const cgpui::Color root_color = cgpui::rgb(40, 50, 60);
  const cgpui::Color child_color = cgpui::rgb(70, 80, 90);

  bool child_context_theme = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        if (context.window_runtime_id == opened.runtime_id) {
          child_context_theme = color_matches(
              context.theme_color(accent), child_color) &&
              context.theme_spacing(gap) == 6.0F &&
              context.window_theme() != nullptr;
        }
      });

  bool app_redraws_all = false;
  bool child_redraw_isolated = false;
  bool root_redraw_isolated = false;
  bool clear_redraw_isolated = false;
  bool lookup_isolated = false;
  application.on_run = [&] {
    cgpui::Theme app_theme;
    app_theme.set_color(accent, app_color).set_spacing(gap, 6.0F);
    runtime.set_app_theme(std::move(app_theme));
    app_redraws_all = application.root.redraw_count == 1 &&
        application.child.redraw_count == 1;
    application.root.callback(cgpui::WindowRedrawRequested{});
    application.child.callback(cgpui::WindowRedrawRequested{});

    application.root.redraw_count = 0;
    application.child.redraw_count = 0;
    cgpui::Theme child_theme;
    child_theme.set_color(accent, child_color);
    runtime.set_window_theme(opened.runtime_id, std::move(child_theme));
    child_redraw_isolated = application.root.redraw_count == 0 &&
        application.child.redraw_count == 1;
    application.child.callback(cgpui::KeyboardKey{
        .key_code = 65,
        .action = cgpui::KeyAction::pressed});
    application.child.callback(cgpui::WindowRedrawRequested{});

    application.root.redraw_count = 0;
    application.child.redraw_count = 0;
    cgpui::Theme root_theme;
    root_theme.set_color(accent, root_color);
    runtime.set_window_theme(
        runtime.root_window_runtime_id(), std::move(root_theme));
    root_redraw_isolated = application.root.redraw_count == 1 &&
        application.child.redraw_count == 0;
    lookup_isolated = color_matches(
        runtime.theme_color(runtime.root_window_runtime_id(), accent),
        root_color) && color_matches(
        runtime.theme_color(opened.runtime_id, accent), child_color) &&
        runtime.theme_spacing(opened.runtime_id, gap) == 6.0F;
    application.root.callback(cgpui::WindowRedrawRequested{});

    application.root.redraw_count = 0;
    application.child.redraw_count = 0;
    clear_redraw_isolated = runtime.clear_window_theme(opened.runtime_id) &&
        application.root.redraw_count == 0 &&
        application.child.redraw_count == 1 &&
        color_matches(runtime.theme_color(opened.runtime_id, accent), app_color);
  };

  const int result = runtime.run(
      cgpui::WindowDescriptor{
          .title = "Root",
          .size = {.width = 640.0F, .height = 480.0F}},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  return result == 0 && app_redraws_all && child_redraw_isolated &&
          root_redraw_isolated && clear_redraw_isolated && lookup_isolated &&
          child_context_theme && renderers.size() == 2
      ? 0
      : 1;
}
