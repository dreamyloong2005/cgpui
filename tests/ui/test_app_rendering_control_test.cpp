#include "cgpui/ui/test_app.hpp"

#include <memory>

namespace {

class RenderView final : public cgpui::View {
 public:
  explicit RenderView(float offset) : offset_(offset) {}

  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_count += 1;
    paint_list.fill_rect(
        cgpui::Rect{.origin = {offset_, offset_}, .size = {8.0F, 6.0F}},
        cgpui::Color{.r = 0.2F, .g = 0.4F, .b = 0.6F, .a = 1.0F});
  }

  int paint_count = 0;

 private:
  float offset_ = 0.0F;
};

bool rendered_once(const cgpui::TestWindowRenderSnapshot& snapshot) {
  return snapshot.begin_frame_count == 1 && snapshot.clear_count == 1 &&
      snapshot.draw_rect_count == 1 && snapshot.present_count == 1;
}

} // namespace

int main() {
  cgpui::TestApp app;
  cgpui::TestAppWindow first = app.open_window(
      cgpui::WindowOptions{}.title("First Render"),
      std::make_unique<RenderView>(1.0F));
  cgpui::TestAppWindow second = app.open_window(
      cgpui::WindowOptions{}.title("Second Render"),
      std::make_unique<RenderView>(2.0F));
  RenderView* first_view = first.root_view_as<RenderView>();
  RenderView* second_view = second.root_view_as<RenderView>();
  if (first_view == nullptr || second_view == nullptr) return 1;
  if (first.render_snapshot().begin_frame_count != 0 ||
      second.render_snapshot().begin_frame_count != 0) {
    return 2;
  }

  first.simulate_resize({40.0F, 30.0F}, cgpui::DpiScale{2.0F});
  const cgpui::TestWindowRenderSnapshot resized = first.render_snapshot();
  if (resized.resize_count != 1 ||
      second.render_snapshot().resize_count != 0 ||
      first.viewport_size().width != 20.0F ||
      first.viewport_size().height != 15.0F || first.scale().value != 2.0F) {
    return 3;
  }

  first.request_redraw();
  if (!rendered_once(first.render_snapshot()) || first_view->paint_count != 1 ||
      second.render_snapshot().begin_frame_count != 0 ||
      second_view->paint_count != 0) {
    return 4;
  }

  if (!second.try_draw_frame()) return 5;
  if (!rendered_once(second.render_snapshot()) || second_view->paint_count != 1 ||
      first.render_snapshot().begin_frame_count != 1) {
    return 6;
  }

  first.draw_frame();
  const cgpui::TestWindowRenderSnapshot final = first.render_snapshot();
  if (final.begin_frame_count != 2 || final.clear_count != 2 ||
      final.draw_rect_count != 2 || final.present_count != 2 ||
      first_view->paint_count != 2) {
    return 7;
  }
  return 0;
}
