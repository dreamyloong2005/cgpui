// Platform accessibility actions (UIA ExpandCollapse / SelectionItem) on
// tree rows and tabs reach the widgets through the window's element tree.
#include "cgpui/ui/tab_strip.hpp"
#include "cgpui/ui/tree_view.hpp"

#include "workbench_test_support.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace {

using Kind = cgpui::AccessibilityActionKind;

struct Fixture {
  cgpui::TestApp app;
  cgpui::TreeView tree{cgpui::TreeViewOptions{.multi_select = true}};
  cgpui::TabStrip tabs;
  std::vector<cgpui::TabStripEvent> tab_events;
  cgpui::TestAppWindow window;
  workbench_test::HostView* view = nullptr;

  Fixture()
      : window(app.open_window(cgpui::WindowOptions{}.size(600.0F, 400.0F),
                               std::make_unique<workbench_test::HostView>())) {
    tree.set_roots({cgpui::TreeItem{.id = 1, .label = "src", .has_children = true},
                    cgpui::TreeItem{.id = 2, .label = "README.md"}});
    tree.set_children(1, {cgpui::TreeItem{.id = 3, .label = "main.cpp"}});
    tabs.set_tabs({cgpui::TabItem{.id = 10, .title = "main.cpp"},
                   cgpui::TabItem{.id = 11, .title = "app.cpp"}});
    tabs.set_selected(10);
    tabs.on_event([this](const cgpui::TabStripEvent& event) { tab_events.push_back(event); });
    view = window.root_view_as<workbench_test::HostView>();
    view->build = [this](cgpui::WidgetTreeBuilder& builder, const cgpui::ViewContext& context) {
      tree.layout(cgpui::Rect{{0, 0}, {200, 400}}, context.scale);
      tabs.layout(cgpui::Rect{{200, 0}, {400, 34}}, context.scale,
                  cgpui::workbench_theme(cgpui::SystemAppearance{}, nullptr));
      tree.append_elements(builder, builder.root());
      tabs.append_elements(builder, builder.root());
    };
    window.draw_frame();
  }

  // Element id of the published node named `name` with `role`.
  [[nodiscard]] std::uint64_t node(cgpui::PlatformAccessibilityRole role,
                                   std::string_view name) const {
    const auto* record = view->runtime->window_runtime_record(window.runtime_id());
    if (record == nullptr || !record->last_accessibility_update.has_value()) return 0;
    for (const auto& entry : record->last_accessibility_update->nodes) {
      if (entry.role == role && entry.name == name) return entry.element_id;
    }
    return 0;
  }

  void act(Kind kind, std::uint64_t element) {
    (void)window.dispatch_platform_event(cgpui::PlatformEvent{
        cgpui::AccessibilityActionRequested{.kind = kind, .element_id = element}});
    window.draw_frame();
  }
};

} // namespace

int main() {
  Fixture f;
  using Role = cgpui::PlatformAccessibilityRole;
  const std::uint64_t src = f.node(Role::tree_item, "src");
  if (src == 0) return 1;
  f.act(Kind::expand, src);
  if (!f.tree.expanded(1) || f.node(Role::tree_item, "main.cpp") == 0) return 2;
  f.act(Kind::collapse, f.node(Role::tree_item, "src"));
  if (f.tree.expanded(1)) return 3;
  f.act(Kind::select, f.node(Role::tree_item, "README.md"));
  if (!f.tree.selected(2) || f.tree.selected(1)) return 4;
  f.act(Kind::add_to_selection, f.node(Role::tree_item, "src"));
  if (!f.tree.selected(1) || !f.tree.selected(2)) return 5;
  f.act(Kind::remove_from_selection, f.node(Role::tree_item, "README.md"));
  if (f.tree.selected(2) || !f.tree.selected(1)) return 6;

  f.act(Kind::select, f.node(Role::tab, "app.cpp"));
  if (f.tab_events.size() != 1 || f.tab_events[0].kind != cgpui::TabStripEventKind::select ||
      f.tab_events[0].tab != 11) {
    return 7;
  }
  // The published state follows the widgets after the redraw.
  const auto* record = f.view->runtime->window_runtime_record(f.window.runtime_id());
  for (const auto& entry : record->last_accessibility_update->nodes) {
    if (entry.element_id == f.node(Role::tree_item, "src") &&
        entry.patterns.selected != true) {
      return 8;
    }
  }
  return 0;
}
