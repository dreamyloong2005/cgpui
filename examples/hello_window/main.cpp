#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/ui.hpp"

#include <cstdlib>
#include <expected>
#include <iostream>
#include <memory>
#include <string_view>

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
  const char* resize_after_first_frame_env =
      std::getenv("CGPUI_RESIZE_AFTER_FIRST_FRAME");
  const bool resize_after_first_frame =
      resize_after_first_frame_env != nullptr &&
      std::string_view(resize_after_first_frame_env) != "0";
  const char* close_after_first_frame_env =
      std::getenv("CGPUI_CLOSE_AFTER_FIRST_FRAME");
  const bool close_after_first_frame =
      close_after_first_frame_env != nullptr &&
      std::string_view(close_after_first_frame_env) != "0";

  auto app = cgpui::create_platform_application();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }

  HelloView view;
  cgpui::Size viewport_size{960.0F, 640.0F};
  bool render_failed = false;
  bool first_frame_presented = false;
  bool resize_requested_after_first_frame = false;
  bool second_frame_presented = false;
  bool close_requested_after_first_frame = false;
  std::unique_ptr<cgpui::Renderer> renderer;

  cgpui::WindowRuntime runtime(
      **app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor& descriptor) {
        auto renderer_result = cgpui::create_renderer(descriptor);
        if (!renderer_result) {
          return cgpui::Result<cgpui::Renderer*>{
              std::unexpected(renderer_result.error())};
        }
        renderer = std::move(*renderer_result);
        return cgpui::Result<cgpui::Renderer*>{renderer.get()};
      });
  runtime.set_error_callback([](const cgpui::Error& error) {
    std::cerr << error.message << '\n';
  });
  runtime.set_close_requested_callback(
      [&](const cgpui::WindowRuntimeContext&) {
        if (close_after_first_frame && first_frame_presented) {
          close_requested_after_first_frame = true;
        }
      });
  runtime.set_after_frame_callback(
      [&](const cgpui::WindowRuntimeContext& context) {
        viewport_size = context.viewport_size;
        if (!first_frame_presented) {
          first_frame_presented = true;
          if (close_after_first_frame) {
            context.window.request_close();
            return;
          }
          if (resize_after_first_frame) {
            viewport_size =
                cgpui::Size{viewport_size.width * 0.75F,
                            viewport_size.height * 0.75F};
            auto resized_result = context.runtime.resize_surface(
                viewport_size,
                cgpui::DpiScale{1.0F});
            if (!resized_result) {
              std::cerr << resized_result.error().message << '\n';
              render_failed = true;
              context.application.quit();
              return;
            }
            resize_requested_after_first_frame = true;
            context.window.request_redraw();
            return;
          }
        } else {
          second_frame_presented = true;
        }
        if (exit_after_first_frame ||
            (resize_after_first_frame && second_frame_presented)) {
          context.application.quit();
        }
      });

  const int run_result = runtime.run(cgpui::WindowDescriptor{
      .title = "CGPUI Hello Window",
      .size = viewport_size});
  if (render_failed) {
    return 1;
  }
  if (exit_after_first_frame && !first_frame_presented) {
    std::cerr << "first frame was not presented\n";
    return 1;
  }
  if (resize_after_first_frame &&
      (!resize_requested_after_first_frame || !second_frame_presented)) {
    std::cerr << "resize smoke did not present a second frame\n";
    return 1;
  }
  if (close_after_first_frame && !close_requested_after_first_frame) {
    std::cerr << "close smoke did not receive a close request\n";
    return 1;
  }
  return run_result;
}
