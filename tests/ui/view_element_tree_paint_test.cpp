// The element tree a view returns from render() is laid out and painted
// every frame, after View::paint, in the root window and in additional
// (TestApp) windows.
#include "window_runtime_test_support.hpp"

#include "cgpui/ui/test_app.hpp"

#include <memory>

namespace {

constexpr cgpui::Color panel_color = cgpui::rgb(31, 38, 46);

class ElementOnlyView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_count += 1;
    if (paint_background) {
      paint_list.fill_rect(cgpui::Rect{{0, 0}, {4, 4}}, cgpui::rgb(1, 2, 3));
    }
  }

  cgpui::AnyElement render(cgpui::ViewContext&) override {
    return cgpui::into_element(cgpui::div()
                                   .size(200.0F, 100.0F)
                                   .padding(cgpui::edges(10.0F))
                                   .child(cgpui::div()
                                              .size(50.0F, 20.0F)
                                              .background(panel_color)));
  }

  int paint_count = 0;
  bool paint_background = false;
};

bool same_color(cgpui::Color lhs, cgpui::Color rhs) {
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

int test_root_window_paints_rendered_elements_after_view_paint() {
  RuntimeFixture fixture;
  fixture.app.on_run = +[] {};
  ElementOnlyView view;
  view.paint_background = true;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  if (runtime.run(cgpui::WindowDescriptor{}) != 0) return 1;
  if (view.paint_count != 1) return 2;
  // View::paint's rect first, then the element's background on top.
  if (fixture.frame.draw_count != 2) return 3;
  const cgpui::SolidRect& last = fixture.frame.last_rect;
  if (!same_color(last.color, panel_color)) return 4;
  if (last.rect.origin.x != 10.0F || last.rect.origin.y != 10.0F ||
      last.rect.size.width != 50.0F || last.rect.size.height != 20.0F) {
    return 5;
  }
  return 0;
}

int test_additional_window_paints_rendered_elements() {
  cgpui::TestApp app;
  cgpui::TestAppWindow window = app.open_window(
      cgpui::WindowOptions{}.size(300.0F, 200.0F),
      std::make_unique<ElementOnlyView>());
  window.draw_frame();
  if (window.render_snapshot().draw_rect_count != 1) return 10;
  return 0;
}

} // namespace

int main() {
  if (const int r = test_root_window_paints_rendered_elements_after_view_paint()) {
    return r;
  }
  return test_additional_window_paints_rendered_elements();
}
