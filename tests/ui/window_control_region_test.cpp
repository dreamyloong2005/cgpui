#include "cgpui/ui/ui.hpp"
#include "runtime_window_title_bar_internal.hpp"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

using cgpui::Rect;
using cgpui::WindowControlArea;
using cgpui::WindowControlRegion;

int fail(int code, const char* message) {
  std::cerr << "window control regions: " << message << '\n';
  return code;
}

bool same(Rect a, Rect b) {
  return std::fabs(a.origin.x - b.origin.x) < 0.01F &&
      std::fabs(a.origin.y - b.origin.y) < 0.01F &&
      std::fabs(a.size.width - b.size.width) < 0.01F &&
      std::fabs(a.size.height - b.size.height) < 0.01F;
}

class RegionWindow final : public cgpui::PlatformWindow {
 public:
  bool client_mode = true;
  int publish_count = 0;
  std::vector<WindowControlRegion> regions;

  cgpui::NativeSurfaceHandle native_surface() const override { return {}; }
  cgpui::WindowState state() const override { return {}; }
  void request_redraw() override {}
  void request_close() override {}
  void set_title(std::string_view) override {}
  void set_cursor(cgpui::CursorShape) override {}
  void set_ime_text_input_placement(
      std::optional<cgpui::ImeTextInputPlacement>) override {}
  void set_window_control_regions(
      std::vector<WindowControlRegion> value) override {
    ++publish_count;
    regions = std::move(value);
  }
  cgpui::PlatformWindowFrameMetrics frame_metrics() const override {
    return {.supported = true, .client_title_bar = client_mode, .backend = "fake"};
  }
};

class CountingFrame final : public cgpui::RenderFrame {
 public:
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { return {}; }
};

class FrameRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unique_ptr<cgpui::RenderFrame>(new CountingFrame);
  }
};

class TitleBarView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_list.add_window_control_region(
        WindowControlArea::close_button, Rect{{360, 0}, {40, 32}});
  }
};

cgpui::AnyElement title_bar_tree() {
  return cgpui::into_element(
      cgpui::div()
          .size(400.0F, 32.0F)
          .bg(cgpui::Color{.r = 0.2F})
          .window_control_area(WindowControlArea::caption)
          .child(cgpui::div()
                     .size(80.0F, 24.0F)
                     .bg(cgpui::Color{.b = 0.8F})
                     .window_control_area(WindowControlArea::client)));
}

} // namespace

int main() {
  // Bounds follow the current transform and are clipped like paint commands.
  cgpui::PaintList list;
  list.push_metadata(cgpui::PaintMetadata{
      .transform = cgpui::AffineTransform::translation(10.0F, 5.0F)});
  list.push_clip(Rect{{0, 0}, {50, 50}});
  list.add_window_control_region(WindowControlArea::caption, Rect{{0, 0}, {100, 20}});
  list.add_window_control_region(WindowControlArea::client, Rect{{200, 0}, {10, 10}});
  list.pop_clip();
  list.pop_metadata();
  if (list.window_control_regions().size() != 1 ||
      !same(list.window_control_regions()[0].bounds, Rect{{10, 5}, {50, 20}})) {
    return fail(1, "regions must be transformed and clipped, empty ones dropped");
  }
  list.clear();
  if (!list.window_control_regions().empty()) return fail(2, "clear drops regions");

  // Elements declare regions while painting; bounds match their painted box,
  // and stay logical whatever the paint scale is.
  cgpui::ElementTree tree;
  (void)tree.set_root(title_bar_tree());
  (void)tree.layout_root(cgpui::LayoutInput{
      .constraints = {.max_size = {400, 300}}, .scale = cgpui::DpiScale{1.5F}});
  cgpui::PaintList painted;
  painted.set_scale(cgpui::DpiScale{1.5F});
  tree.paint(painted);
  std::vector<Rect> boxes;
  for (const cgpui::PaintCommand& command : painted.commands()) {
    if (command.kind == cgpui::PaintCommandKind::solid_rect) {
      boxes.push_back(command.solid_rect.rect);
    }
  }
  const auto regions = painted.window_control_regions();
  if (regions.size() != 2 || boxes.size() != 2 ||
      regions[0].area != WindowControlArea::caption ||
      regions[1].area != WindowControlArea::client ||
      !same(regions[0].bounds, boxes[0]) || !same(regions[1].bounds, boxes[1]) ||
      regions[1].bounds.size.width != 80.0F) {
    for (const auto& region : regions) {
      std::cerr << "region " << region.bounds.origin.x << ','
                << region.bounds.origin.y << ' ' << region.bounds.size.width
                << 'x' << region.bounds.size.height << '\n';
    }
    for (const Rect& box : boxes) {
      std::cerr << "box " << box.origin.x << ',' << box.origin.y << ' '
                << box.size.width << 'x' << box.size.height << '\n';
    }
    return fail(3, "element regions must match the painted boxes in logical pixels");
  }

  // render_view hands the View::paint regions back to the caller.
  FrameRenderer renderer;
  TitleBarView view;
  std::vector<WindowControlRegion> frame_regions;
  if (!cgpui::render_view(renderer, view, {400, 300}, cgpui::DpiScale{2.0F}, nullptr,
                          cgpui::Color{}, &frame_regions, nullptr) ||
      frame_regions.size() != 1 ||
      frame_regions[0].area != WindowControlArea::close_button) {
    return fail(4, "render_view must report the painted regions");
  }

  // The runtime publishes view regions first, then the element tree's.
  RegionWindow window;
  cgpui::runtime_publish_window_control_regions(
      &window, &tree, cgpui::DpiScale{1.5F}, frame_regions);
  if (window.regions.size() != 3 ||
      window.regions[0].area != WindowControlArea::close_button ||
      window.regions[1].area != WindowControlArea::caption ||
      window.regions[2].area != WindowControlArea::client) {
    return fail(5, "client mode must publish view and element regions");
  }
  window.client_mode = false;
  cgpui::runtime_publish_window_control_regions(
      &window, &tree, cgpui::DpiScale{1.5F}, {});
  if (window.publish_count != 2 || !window.regions.empty()) {
    return fail(6, "system mode skips the element pass");
  }

  // The base PlatformWindow keeps working for backends without support.
  const cgpui::PlatformWindowActionResult menu = window.perform_window_action(
      {.action = cgpui::PlatformWindowAction::show_window_menu});
  if (menu.supported || menu.reason.empty()) {
    return fail(7, "unsupported window actions must say why");
  }
  return 0;
}
