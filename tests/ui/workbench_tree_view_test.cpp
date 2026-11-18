#include "cgpui/ui/tree_view.hpp"

#include "workbench_test_support.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

using cgpui::TreeItem;
using cgpui::TreeViewEvent;
using cgpui::TreeViewEventKind;

constexpr cgpui::TreeItemId workspace = 1;
constexpr cgpui::TreeItemId alpha = 10;
constexpr cgpui::TreeItemId beta = 20;
constexpr cgpui::TreeItemId bravo = 30;
constexpr cgpui::TreeItemId charlie = 40;

// Lazily provides children: the workspace has `big` files, alpha has three.
struct Fixture {
  cgpui::TestApp app;
  cgpui::TreeView tree;
  std::vector<TreeViewEvent> events;
  std::size_t big = 0;
  cgpui::TestAppWindow window;

  explicit Fixture(std::size_t big_children = 0)
      : big(big_children),
        window(app.open_window(cgpui::WindowOptions{}.size(400.0F, 440.0F),
                               std::make_unique<workbench_test::HostView>())) {
    auto* view = window.root_view_as<workbench_test::HostView>();
    view->build = [this](cgpui::WidgetTreeBuilder& builder,
                         const cgpui::ViewContext& context) {
      tree.layout(cgpui::Rect{{0, 0}, {400, 440}}, context.scale);
      tree.append_elements(builder, builder.root());
    };
    view->on_event = [this](const cgpui::PlatformEvent& event,
                            const cgpui::ViewContext& context) {
      return tree.handle_event(event, context);
    };
    tree.on_event([this](const TreeViewEvent& event) {
      events.push_back(event);
      if (event.kind == TreeViewEventKind::children_requested) load(event.item);
    });
    if (big > 0) {
      tree.set_roots({TreeItem{workspace, "workspace", true}});
    } else {
      tree.set_roots({TreeItem{alpha, "alpha", true}, TreeItem{beta, "beta"},
                      TreeItem{bravo, "bravo"}, TreeItem{charlie, "charlie"}});
    }
    window.draw_frame();
  }

  void load(cgpui::TreeItemId parent) {
    std::vector<TreeItem> children;
    if (parent == workspace) {
      children.reserve(big);
      for (std::size_t index = 0; index < big; ++index) {
        children.push_back(TreeItem{1000 + index, "file " + std::to_string(index)});
      }
    } else if (parent == alpha) {
      children = {TreeItem{11, "a1"}, TreeItem{12, "a2"}, TreeItem{13, "a3"}};
    }
    tree.set_children(parent, std::move(children));
  }

  [[nodiscard]] cgpui::AccessibilityTreeSnapshot snapshot() {
    return workbench_test::snapshot_of(
        {400.0F, 440.0F}, tree.element_id(), [&](cgpui::WidgetTreeBuilder& builder) {
          tree.append_elements(builder, builder.root());
        });
  }
};

int test_virtualizes_100k_items() {
  Fixture f(100000);
  f.tree.expand(workspace);
  if (f.tree.row_count() != 100001 || !f.tree.children_loaded(workspace)) return 10;
  f.window.draw_frame();
  const cgpui::UniformListVisibleRange range = f.tree.visible_range();
  // 440 px / 22 px = 20 rows on screen, plus 4 rows of overscan below.
  if (range.start_index != 0 || range.end_index != 24) return 11;
  cgpui::AccessibilityTreeSnapshot snapshot = f.snapshot();
  if (workbench_test::count_role(snapshot, cgpui::AccessibilityRole::tree_item) != 24) {
    return 12;
  }
  f.tree.scroll_to(50000.0F * 22.0F);
  const cgpui::UniformListVisibleRange middle = f.tree.visible_range();
  if (middle.start_index != 49996 || middle.end_index != 50024) return 13;
  snapshot = f.snapshot();
  const auto* first =
      workbench_test::find_role(snapshot, cgpui::AccessibilityRole::tree_item);
  if (workbench_test::count_role(snapshot, cgpui::AccessibilityRole::tree_item) != 28 ||
      first == nullptr || first->name != "file 49995" || first->patterns.level != 2) {
    return 14;
  }
  // The last row is reachable and nothing beyond it is produced.
  f.tree.scroll_to(1.0e9F);
  if (f.tree.visible_range().end_index != 100001) return 15;
  return 0;
}

int test_keyboard_navigation() {
  using workbench_test::key;
  Fixture f;
  f.window.focus(f.tree.element_id());
  key(f.window, 40);  // Down: first row
  if (f.tree.focused_item() != alpha || !f.tree.selected(alpha)) return 20;
  key(f.window, 39);  // Right: expand, children load lazily
  if (!f.tree.expanded(alpha) || f.tree.row_count() != 7) return 21;
  if (f.events.size() < 2 ||
      f.events[f.events.size() - 1].kind != TreeViewEventKind::children_requested) {
    return 22;
  }
  key(f.window, 39);  // Right: first child
  if (f.tree.focused_item() != 11) return 23;
  key(f.window, 40);
  key(f.window, 37);  // Left on a leaf: parent
  if (f.tree.focused_item() != alpha) return 24;
  key(f.window, 37);  // Left on an expanded item: collapse
  if (f.tree.expanded(alpha) || f.tree.row_count() != 4) return 25;
  key(f.window, 35);
  if (f.tree.focused_item() != charlie) return 26;
  key(f.window, 36);
  key(f.window, 40, {.shift = true});
  key(f.window, 40, {.shift = true});
  if (f.tree.selection() != std::vector<cgpui::TreeItemId>{alpha, beta, bravo}) {
    return 27;
  }
  key(f.window, 13);
  if (f.events.back().kind != TreeViewEventKind::activated ||
      f.events.back().item != bravo) {
    return 28;
  }
  return 0;
}

void modifier(const cgpui::TestAppWindow& window, std::uint32_t code, bool down) {
  window.dispatch_keystroke(cgpui::KeyboardKey{
      .key_code = code,
      .action = down ? cgpui::KeyAction::pressed : cgpui::KeyAction::released,
      .modifiers = {.shift = down && code == 16, .control = down && code == 17}});
}

int test_pointer_selection() {
  using workbench_test::center;
  using workbench_test::click;
  Fixture f;
  click(f.window, center(f.tree.row_rect(1)));
  if (f.tree.selection() != std::vector<cgpui::TreeItemId>{beta}) return 30;
  if (f.window.input_state().keyboard_focus_element_owner != f.tree.element_id()) {
    return 31;
  }
  modifier(f.window, 16, true);
  click(f.window, center(f.tree.row_rect(3)));
  modifier(f.window, 16, false);
  if (f.tree.selection() != std::vector<cgpui::TreeItemId>{beta, bravo, charlie}) {
    return 32;
  }
  modifier(f.window, 17, true);
  click(f.window, center(f.tree.row_rect(2)));
  modifier(f.window, 17, false);
  if (f.tree.selection() != std::vector<cgpui::TreeItemId>{beta, charlie}) return 33;
  // The twisty toggles expansion without changing the selection.
  const cgpui::Rect first = f.tree.row_rect(0);
  click(f.window, {first.origin.x + 10.0F, first.origin.y + 11.0F});
  if (!f.tree.expanded(alpha) || f.tree.selected(alpha)) return 34;
  click(f.window, center(f.tree.row_rect(1)), cgpui::MouseButton::left, 2);
  if (f.events.back().kind != TreeViewEventKind::activated || f.events.back().item != 11) {
    return 35;
  }
  const cgpui::Point menu_at = center(f.tree.row_rect(4));
  (void)f.window.dispatch_platform_event(cgpui::PointerButton{
      .button = cgpui::MouseButton::right, .pressed = true, .position = menu_at});
  if (f.events.back().kind != TreeViewEventKind::context_menu_requested ||
      f.events.back().item != beta || f.events.back().position.y != menu_at.y ||
      f.tree.selection() != std::vector<cgpui::TreeItemId>{beta, charlie}) {
    return 36;  // right-click on a selected row keeps the multi-selection
  }
  return 0;
}

int test_stable_identity() {
  Fixture f;
  f.tree.expand(alpha);
  f.tree.select(12);
  f.tree.set_children(alpha, {TreeItem{13, "a3"}, TreeItem{12, "a2 renamed"},
                              TreeItem{14, "a4"}});
  if (!f.tree.selected(12) || f.tree.focused_item() != 12 || f.tree.row_of(12) != 2 ||
      f.tree.contains(11) || f.tree.row_count() != 7) {
    return 40;
  }
  // Removing the selected item drops it from the selection and focus.
  f.tree.set_children(alpha, {TreeItem{13, "a3"}});
  if (f.tree.selected(12) || f.tree.focused_item().has_value()) return 41;
  // Collapsing keeps the loaded children for the next expand.
  f.tree.collapse(alpha);
  const std::size_t requests = f.events.size();
  f.tree.expand(alpha);
  for (std::size_t index = requests; index < f.events.size(); ++index) {
    if (f.events[index].kind == TreeViewEventKind::children_requested) return 42;
  }
  return 0;
}

int test_type_to_select() {
  Fixture f;
  f.window.focus(f.tree.element_id());
  (void)f.window.dispatch_platform_event(cgpui::TextInput{.text = "b"});
  if (f.tree.focused_item() != beta) return 50;
  (void)f.window.dispatch_platform_event(cgpui::TextInput{.text = "r"});
  if (f.tree.focused_item() != bravo) return 51;
  f.app.advance_time(1000);
  (void)f.window.dispatch_platform_event(cgpui::TextInput{.text = "c"});
  if (f.tree.focused_item() != charlie) return 52;
  // Within the timeout the next character extends the prefix ("cz").
  (void)f.window.dispatch_platform_event(cgpui::TextInput{.text = "z"});
  if (f.tree.focused_item() != charlie) return 53;
  return 0;
}

int test_accessibility() {
  Fixture f;
  f.tree.expand(alpha);
  f.tree.select(12);
  const cgpui::AccessibilityTreeSnapshot snapshot = f.snapshot();
  const auto* tree = workbench_test::find_role(snapshot, cgpui::AccessibilityRole::tree);
  const auto* parent =
      workbench_test::find_role(snapshot, cgpui::AccessibilityRole::tree_item, 0);
  const auto* child =
      workbench_test::find_role(snapshot, cgpui::AccessibilityRole::tree_item, 2);
  const auto* leaf =
      workbench_test::find_role(snapshot, cgpui::AccessibilityRole::tree_item, 4);
  if (tree == nullptr || !tree->focusable || tree->children.size() != 7) return 60;
  if (parent == nullptr || parent->patterns.expanded != true ||
      parent->patterns.level != 1 || parent->patterns.selected != false) {
    return 61;
  }
  if (child == nullptr || child->name != "a2" || child->patterns.level != 2 ||
      child->patterns.selected != true || child->patterns.expanded.has_value()) {
    return 62;
  }
  if (leaf == nullptr || leaf->name != "beta" || leaf->patterns.level != 1) return 63;
  return 0;
}

} // namespace

int main() {
  for (const auto test : {test_virtualizes_100k_items, test_keyboard_navigation,
                          test_pointer_selection, test_stable_identity,
                          test_type_to_select, test_accessibility}) {
    if (const int result = test(); result != 0) {
      std::cerr << "workbench_tree_view_test failed: " << result << '\n';
      return result;
    }
  }
  return 0;
}
