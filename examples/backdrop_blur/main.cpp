// A floating panel with a box shadow and a backdrop blur over high-contrast
// stripes and text. Environment:
//   CGPUI_EXAMPLE_NO_BLUR=1         panel without the backdrop blur
//   CGPUI_EXAMPLE_BENCH_FRAMES=N    repaint N frames, print mean renderer
//                                   stage timings, then quit
#include "cgpui/cgpui.hpp"

#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

constexpr cgpui::Size window_size{900.0F, 600.0F};

class BackdropView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size viewport) override {
    using cgpui::Rect;
    paint_list.fill_rect(Rect{{0, 0}, viewport}, cgpui::rgb(250, 250, 250));
    const cgpui::Color colors[]{cgpui::rgb(20, 20, 20), cgpui::rgb(220, 40, 40),
                                cgpui::rgb(30, 110, 230), cgpui::rgb(250, 200, 20)};
    for (int index = 0; index * 12.0F < viewport.width; ++index) {
      if (index % 2 == 1) continue;
      paint_list.fill_rect(Rect{{index * 12.0F, 0}, {6.0F, viewport.height}},
                           colors[(index / 2) % 4]);
    }
    for (int row = 0; row < 6; ++row) {
      paint_list.fill_text(
          Rect{{40, 60.0F + row * 90.0F}, {820, 60}}, cgpui::rgb(0, 0, 0),
          "CGPUI backdrop blur", {}, 48.0F);
    }
    const Rect panel{{240, 170}, {420, 260}};
    const cgpui::BorderRadii radius = cgpui::BorderRadii::all(18.0F);
    paint_list.draw_box_shadow(
        panel, cgpui::BoxShadow{.color = cgpui::rgba(0, 0, 0, 0.45F),
                                .offset = {0, 16},
                                .blur_radius = 48.0F},
        radius);
    if (std::getenv("CGPUI_EXAMPLE_NO_BLUR") == nullptr) {
      paint_list.draw_backdrop_blur(panel, radius, 20.0F,
                                    cgpui::rgba(255, 255, 255, 0.35F));
    }
    paint_list.fill_stroked_rounded_rect(
        panel, cgpui::rgba(255, 255, 255, 0.25F), radius,
        cgpui::rgba(255, 255, 255, 0.6F), 1.0F);
    paint_list.fill_text(Rect{{272, 200}, {360, 40}}, cgpui::rgb(10, 10, 10),
                         "Floating panel", {}, 28.0F);
  }
};

void report(const cgpui::ViewContext& context, int frames) {
  static int seen = 0;
  static cgpui::RendererFrameTimings sum;
  const auto snapshot = context.diagnostics_snapshot();
  if (snapshot.last_renderer_frame_diagnostics.has_value() && seen++ >= 10) {
    const auto& t = snapshot.last_renderer_frame_diagnostics->timings;
    sum.pacing_wait_nanoseconds += t.pacing_wait_nanoseconds;
    sum.resource_preparation_nanoseconds += t.resource_preparation_nanoseconds;
    sum.command_recording_nanoseconds += t.command_recording_nanoseconds;
    sum.total_nanoseconds += t.total_nanoseconds;
  }
  if (seen < frames + 10) {
    context.request_paint();
    return;
  }
  const auto ms = [&](std::uint64_t value) {
    return static_cast<double>(value) / frames / 1e6;
  };
  const auto& blur = snapshot.last_renderer_frame_diagnostics->backdrop_blur;
  std::printf("frames=%d total=%.3fms pacing_wait=%.3fms prep=%.3fms "
              "record=%.3fms offscreen_passes=%zu\n",
              frames, ms(sum.total_nanoseconds), ms(sum.pacing_wait_nanoseconds),
              ms(sum.resource_preparation_nanoseconds),
              ms(sum.command_recording_nanoseconds), blur.offscreen_pass_count);
  context.application.quit();
}

} // namespace

int main() {
  auto app = cgpui::Application::create();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }
  BackdropView view;
  cgpui::AppRunnerOptions options;
  options.window = cgpui::WindowOptions{}
                       .title("CGPUI backdrop blur")
                       .size(window_size)
                       .to_descriptor();
  options.setup_context = [](cgpui::AppContext& app_context) {
    app_context.runtime.set_after_frame_callback(
        [](const cgpui::ViewContext& context) {
          if (const char* frames = std::getenv("CGPUI_EXAMPLE_BENCH_FRAMES")) {
            report(context, std::max(1, std::atoi(frames)));
          } else if (std::getenv("CGPUI_EXIT_AFTER_FIRST_FRAME") != nullptr) {
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
