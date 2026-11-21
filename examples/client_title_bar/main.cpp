// Client-drawn title bar: the view paints its own caption row and declares
// the caption, a non-dragging control, and the three caption buttons as
// window control regions. The platform keeps native move, resize, snap,
// double-click maximize and the system menu; the buttons act on release.
#include "cgpui/cgpui.hpp"

#include <cstdlib>
#include <iostream>
#include <variant>

namespace {

constexpr float caption_height = 32.0F;
constexpr float button_width = 46.0F;

class TitleBarView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size viewport) override {
    using cgpui::Rect;
    using cgpui::WindowControlArea;
    const float width = viewport.width;
    paint_list.fill_rect(Rect{{0, 0}, {width, caption_height}}, cgpui::rgb(32, 34, 38));
    paint_list.add_window_control_region(
        WindowControlArea::caption, Rect{{0, 0}, {width, caption_height}});
    const Rect command_box{{width / 2 - 120, 4}, {240, caption_height - 8}};
    paint_list.fill_rect(command_box, cgpui::rgb(52, 55, 61));
    paint_list.add_window_control_region(WindowControlArea::client, command_box);
    // macOS keeps its native traffic lights at the left of the caption row.
    if (leading_reserved_ > 0.0F) return;
    const WindowControlArea buttons[]{
        WindowControlArea::minimize_button, WindowControlArea::maximize_button,
        WindowControlArea::close_button};
    for (int index = 0; index < 3; ++index) {
      const Rect bounds{
          {width - button_width * static_cast<float>(3 - index), 0},
          {button_width, caption_height}};
      const bool hovered = cgpui::contains(bounds, pointer_);
      const bool close = buttons[index] == WindowControlArea::close_button;
      paint_list.fill_rect(
          bounds, hovered ? (close ? cgpui::rgb(196, 43, 28) : cgpui::rgb(62, 65, 72))
                          : cgpui::rgb(32, 34, 38));
      paint_list.add_window_control_region(buttons[index], bounds);
    }
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    // Over caption buttons Windows forwards WM_NCMOUSEMOVE as PointerMoved.
    if (const auto* moved = std::get_if<cgpui::PointerMoved>(&event)) {
      pointer_ = moved->position;
      context.request_paint();
    } else if (std::holds_alternative<cgpui::PointerExited>(event)) {
      pointer_ = {-1, -1};
      context.request_paint();
    }
    return cgpui::EventResult::unhandled();
  }

  // Width the platform's own caption buttons keep at the left (macOS).
  // Returns true when it changed and the view must be painted again.
  bool set_leading_reserved(float width) {
    if (width == leading_reserved_) return false;
    leading_reserved_ = width;
    return true;
  }

 private:
  cgpui::Point pointer_{-1, -1};
  float leading_reserved_ = 0.0F;
};

} // namespace

int main() {
  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }
  TitleBarView view;
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI client title bar")
                       .size(cgpui::Size{900.0F, 600.0F})
                       .title_bar(cgpui::WindowTitleBarMode::client)
                       .backdrop(std::getenv("CGPUI_EXAMPLE_BACKDROP") != nullptr
                                     ? cgpui::WindowBackdrop::mica
                                     : cgpui::WindowBackdrop::none)
                       .to_descriptor();
  options.setup_context = [&view](cgpui::AppContext& app_context) {
    app_context.runtime.set_after_frame_callback(
        [&view](const cgpui::ViewContext& context) {
          if (view.set_leading_reserved(context.platform_window.frame_metrics()
                                            .caption_buttons_leading_reserved_width)) {
            context.request_paint();
            return;
          }
          if (std::getenv("CGPUI_EXIT_AFTER_FIRST_FRAME") != nullptr) {
            context.application.quit();
          }
        });
  };
  return app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      options);
}
