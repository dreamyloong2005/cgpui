#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "cgpui/ui/ui.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string_view>
#include <variant>

class HelloView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size viewport_size) override {
    const auto rect_size =
        cgpui::Size{viewport_size.width * 0.35F, viewport_size.height * 0.25F};
    paint_list.fill_rect(
        cgpui::Rect{
            .origin = {viewport_size.width * 0.325F,
                       viewport_size.height * 0.375F},
            .size = rect_size},
        cgpui::Color{.r = 0.23F, .g = 0.55F, .b = 0.86F, .a = 1.0F});
  }
};

int main() {
  const char* exit_after_first_frame_env =
      std::getenv("CGPUI_EXIT_AFTER_FIRST_FRAME");
  const bool exit_after_first_frame =
      exit_after_first_frame_env != nullptr &&
      std::string_view(exit_after_first_frame_env) != "0";

  auto app = cgpui::create_platform_application();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  HelloView view;
  cgpui::Size viewport_size{960.0F, 640.0F};
  bool should_quit = false;
  bool render_failed = false;
  bool first_frame_presented = false;
  std::unique_ptr<cgpui::PlatformWindow> window;
  std::unique_ptr<cgpui::Renderer> renderer;

  auto window_result = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Hello Window",
                              .size = viewport_size},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          should_quit = true;
          (*app)->quit();
          return;
        }

        if (const auto* resized = std::get_if<cgpui::WindowResized>(&event);
            resized != nullptr) {
          viewport_size = resized->size;
          if (renderer) {
            auto resized_result = renderer->resize(resized->size,
                                                   resized->scale);
            if (!resized_result) {
              std::cerr << resized_result.error().message << '\n';
            }
          }
          return;
        }

        if (std::holds_alternative<cgpui::WindowRedrawRequested>(event)) {
          if (renderer && !should_quit) {
            auto render_result = cgpui::render_view(*renderer, view, viewport_size);
            if (!render_result) {
              std::cerr << render_result.error().message << '\n';
              render_failed = true;
              should_quit = true;
              (*app)->quit();
              return;
            }
            first_frame_presented = true;
            if (exit_after_first_frame) {
              should_quit = true;
              (*app)->quit();
            }
          }
          return;
        }
      });

  if (!window_result) {
    std::cerr << window_result.error().message << '\n';
    return 1;
  }

  window = std::move(*window_result);
  viewport_size = window->state().framebuffer_size;

  auto renderer_result = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window->native_surface(),
      .framebuffer_size = window->state().framebuffer_size,
      .scale = window->state().scale});
  if (!renderer_result) {
    std::cerr << renderer_result.error().message << '\n';
    return 1;
  }

  renderer = std::move(*renderer_result);
  window->request_redraw();
  const int run_result = (*app)->run();
  if (render_failed) {
    return 1;
  }
  if (exit_after_first_frame && !first_frame_presented) {
    std::cerr << "first frame was not presented\n";
    return 1;
  }
  return run_result;
}
