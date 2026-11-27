#include "baseline_measure.hpp"
#include "baseline_workloads.hpp"

#include "cgpui/prelude.hpp"
#include "cgpui/ui/test_app.hpp"

#include <memory>

namespace performance_baseline {
namespace {
class WindowView final : public cgpui::View {
 public:
  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    return cgpui::into_element(
        cgpui::div().size(context.viewport_size).background(cgpui::rgb(20, 30, 40)));
  }
  void paint(cgpui::PaintList& paint, cgpui::Size) override {
    ++paint_count;
    paint.fill_rect({.origin = {1, 1}, .size = {8, 8}}, cgpui::rgb(40, 120, 200));
  }
  std::size_t paint_count = 0;
};
} // namespace

void append_window_samples(std::vector<Sample>& samples, std::size_t iterations) {
  std::size_t startups = 0;
  samples.push_back(measure("startup", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      cgpui::TestApp app;
      if (app.window_count() == 0) ++startups;
    }
  }, [&] { return WorkloadObservation{startups, true}; }));

  std::size_t presented = 0;
  bool frames_valid = true;
  samples.push_back(measure("first_frame", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      cgpui::TestApp app;
      auto window = app.open_window({}, std::make_unique<WindowView>());
      const auto result = window.try_draw_frame();
      const auto snapshot = window.render_snapshot();
      const auto* view = window.root_view_as<WindowView>();
      presented += snapshot.present_count;
      frames_valid = frames_valid && result.has_value() && view != nullptr &&
          view->paint_count == 1 && snapshot.begin_frame_count == 1 &&
          snapshot.present_count == 1 && snapshot.draw_rect_count == 2;
    }
  }, [&] { return WorkloadObservation{presented, frames_valid}; }));

  cgpui::TestApp app;
  auto window = app.open_window({}, std::make_unique<WindowView>());
  const auto before = window.render_snapshot().resize_count;
  bool sizes_valid = true;
  samples.push_back(measure("resize", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      const float edge = index % 2 == 0 ? 640.0F : 800.0F;
      window.simulate_resize({edge, edge * 0.75F}, cgpui::DpiScale{1.0F});
      const auto size = window.viewport_size();
      sizes_valid = sizes_valid && size.width == edge && size.height == edge * 0.75F;
    }
  }, [&] {
    return WorkloadObservation{window.render_snapshot().resize_count - before, sizes_valid};
  }));
}

} // namespace performance_baseline
