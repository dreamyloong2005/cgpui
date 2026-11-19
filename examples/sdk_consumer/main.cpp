// CGPUI SDK consumer sample (docs/sdk-packaging.md). Built outside the
// repository against an extracted SDK package with CMake or xmake: a
// window with a client-drawn title bar and text. With
// CGPUI_EXIT_AFTER_FIRST_FRAME=1 it exits after the first presented frame
// and returns 0 only if that frame was presented.
#include "cgpui/cgpui.hpp"

#include <cstdlib>
#include <iostream>

namespace {

constexpr float caption_height = 32.0F;
constexpr float button_width = 46.0F;

class ConsumerView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size viewport) override {
    using cgpui::Rect;
    using cgpui::WindowControlArea;
    const float width = viewport.width;
    const Rect caption{{0, 0}, {width, caption_height}};
    paint_list.fill_rect(caption, cgpui::rgb(32, 34, 38));
    paint_list.add_window_control_region(WindowControlArea::caption, caption);
    paint_list.fill_text(Rect{{12, 7}, {400, 20}}, cgpui::rgb(220, 224, 230),
                         "CGPUI SDK consumer", {}, 14.0F);
    const WindowControlArea buttons[]{
        WindowControlArea::minimize_button, WindowControlArea::maximize_button,
        WindowControlArea::close_button};
    for (int index = 0; index < 3; ++index) {
      const Rect bounds{
          {width - button_width * static_cast<float>(3 - index), 0},
          {button_width, caption_height}};
      paint_list.add_window_control_region(buttons[index], bounds);
    }
    paint_list.fill_rect(
        Rect{{0, caption_height}, {width, viewport.height - caption_height}},
        cgpui::rgb(230, 240, 247));
    paint_list.fill_text(Rect{{24, 64}, {width - 48, 40}},
                         cgpui::rgb(26, 36, 47), "Hello from the CGPUI SDK",
                         {}, 28.0F);
  }
};

} // namespace

int main() {
  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }
  ConsumerView view;
  const bool exit_after_first_frame =
      std::getenv("CGPUI_EXIT_AFTER_FIRST_FRAME") != nullptr;
  bool presented = false;
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI SDK consumer")
                       .size(cgpui::Size{720.0F, 360.0F})
                       .title_bar(cgpui::WindowTitleBarMode::client)
                       .to_descriptor();
  options.setup_context = [&](cgpui::AppContext& app_context) {
    app_context.runtime.set_after_frame_callback(
        [&](const cgpui::ViewContext& context) {
          presented = true;
          if (exit_after_first_frame) {
            context.application.quit();
          }
        });
  };
  const int result = app->run(
      view,
      [](const cgpui::RenderSurfaceDescriptor& descriptor) {
        return cgpui::create_renderer(descriptor);
      },
      options);
  if (exit_after_first_frame && !presented) {
    std::cerr << "first frame was not presented\n";
    return 1;
  }
  if (presented) {
    std::cout << "sdk_consumer: first frame presented\n";
  }
  return result;
}
