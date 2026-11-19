#pragma once

// Shared host view for the workbench widget tests: the view builds a
// WidgetTreeBuilder tree on every render, forwards events to the widgets
// under test and paints them, the same way an application does.
#include "cgpui/ui/runtime.hpp"
#include "cgpui/ui/test_app.hpp"
#include "cgpui/ui/widget_tree.hpp"

#include <functional>
#include <memory>

namespace workbench_test {

class HostView final : public cgpui::View {
 public:
  std::function<void(cgpui::WidgetTreeBuilder&, const cgpui::ViewContext&)> build;
  std::function<cgpui::EventResult(const cgpui::PlatformEvent&,
                                   const cgpui::ViewContext&)>
      on_event;
  std::function<void(cgpui::PaintList&)> on_paint;
  cgpui::WindowRuntime* runtime = nullptr;
  int render_count = 0;
  int paint_count = 0;

  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    runtime = &context.runtime;
    cgpui::WidgetTreeBuilder builder(context);
    if (build) build(builder, context);
    builder.install(context);
    render_count += 1;
    return {};
  }

  void paint(cgpui::PaintList& paint_list, cgpui::Size) override {
    paint_count += 1;
    if (on_paint) on_paint(paint_list);
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::ViewContext& context) override {
    runtime = &context.runtime;
    return on_event ? on_event(event, context) : cgpui::EventResult::unhandled();
  }
};

[[nodiscard]] inline cgpui::Point center(cgpui::Rect rect) {
  return {rect.origin.x + rect.size.width / 2.0F,
          rect.origin.y + rect.size.height / 2.0F};
}

// Accessibility snapshot of a freshly built widget tree, read from the tree
// the builder produces (window records also publish their installed tree;
// see window_record_element_tree_test).
template <typename Build>
[[nodiscard]] cgpui::AccessibilityTreeSnapshot snapshot_of(
    cgpui::Size viewport, std::optional<cgpui::ElementId> focused, Build&& build) {
  cgpui::WidgetTreeBuilder builder(viewport, focused);
  build(builder);
  const std::unique_ptr<cgpui::ElementTree> tree = builder.finish();
  return tree->accessibility_snapshot(
      cgpui::AccessibilitySnapshotOptions{.focused_element_id = focused});
}

[[nodiscard]] inline const cgpui::AccessibilityNode* find_role(
    const cgpui::AccessibilityTreeSnapshot& snapshot,
    cgpui::AccessibilityRole role,
    std::size_t skip = 0) {
  for (const cgpui::AccessibilityNode& node : snapshot.nodes) {
    if (node.role != role) continue;
    if (skip == 0) return &node;
    skip -= 1;
  }
  return nullptr;
}

[[nodiscard]] inline std::size_t count_role(
    const cgpui::AccessibilityTreeSnapshot& snapshot,
    cgpui::AccessibilityRole role) {
  std::size_t count = 0;
  for (const cgpui::AccessibilityNode& node : snapshot.nodes) {
    if (node.role == role) count += 1;
  }
  return count;
}

inline void key(const cgpui::TestAppWindow& window, std::uint32_t code,
                cgpui::KeyboardModifiers modifiers = {}) {
  window.dispatch_keystroke(cgpui::KeyboardKey{
      .key_code = code, .action = cgpui::KeyAction::pressed,
      .modifiers = modifiers});
}

inline void click(const cgpui::TestAppWindow& window, cgpui::Point point,
                  cgpui::MouseButton button = cgpui::MouseButton::left,
                  std::uint8_t clicks = 1) {
  (void)window.dispatch_platform_event(cgpui::PointerButton{
      .button = button, .pressed = true, .click_count = clicks,
      .position = point});
  (void)window.dispatch_platform_event(cgpui::PointerButton{
      .button = button, .pressed = false, .click_count = clicks,
      .position = point});
}

} // namespace workbench_test
