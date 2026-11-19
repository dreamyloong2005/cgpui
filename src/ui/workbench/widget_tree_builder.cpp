#include "cgpui/ui/widget_tree.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

#include <utility>

namespace cgpui {

WidgetTreeBuilder::WidgetTreeBuilder(
    Size viewport,
    std::optional<ElementId> focused_element)
    : viewport_(viewport), focused_element_(focused_element) {
  pending_.reserve(64);
}

WidgetTreeBuilder::WidgetTreeBuilder(const WindowRuntimeContext& context)
    : WidgetTreeBuilder(
          context.viewport_size,
          context.input_state().keyboard_focus_element_owner) {}

WidgetTreeBuilder::NodeHandle WidgetTreeBuilder::root() const {
  return 0;
}

Size WidgetTreeBuilder::viewport() const {
  return viewport_;
}

std::optional<ElementId> WidgetTreeBuilder::focused_element() const {
  return focused_element_;
}

WidgetTreeBuilder::NodeHandle WidgetTreeBuilder::add(
    NodeHandle parent,
    WidgetNode node,
    bool stable,
    ElementId* id_out) {
  if (parent > pending_.size()) parent = 0;
  pending_.push_back(Pending{
      .parent = parent,
      .node = std::move(node),
      .stable = stable,
      .id_out = id_out,
  });
  return pending_.size();
}

void WidgetTreeBuilder::request_focus(const ElementId* id) {
  focus_request_ = id;
}

std::unique_ptr<ElementTree> WidgetTreeBuilder::finish() {
  cursors_.clear();
  auto tree = std::make_unique<ElementTree>();
  const ElementId root_id = tree->set_root(std::make_unique<WidgetNodeElement>(
      WidgetNode{.bounds = Rect{.size = viewport_}, .hit_testable = true},
      Point{}));

  // Handle h (1-based) maps to ids[h]; ids[0] is the root.
  std::vector<ElementId> ids(pending_.size() + 1);
  ids[0] = root_id;
  const auto append = [&](std::size_t index) {
    Pending& entry = pending_[index];
    const ElementId parent = ids[entry.parent];
    if (parent.value == 0) return false;
    const Point parent_origin = entry.parent == 0
        ? Point{}
        : pending_[entry.parent - 1].node.bounds.origin;
    const std::optional<CursorShape> cursor = entry.node.cursor;
    ids[index + 1] = tree->append_child(
        parent, std::make_unique<WidgetNodeElement>(entry.node, parent_origin));
    if (entry.id_out != nullptr) *entry.id_out = ids[index + 1];
    if (cursor.has_value()) cursors_.emplace_back(ids[index + 1], *cursor);
    return true;
  };
  // Stable nodes whose parents are stable (or the root) come first.
  for (std::size_t index = 0; index < pending_.size(); ++index) {
    if (pending_[index].stable) (void)append(index);
  }
  for (std::size_t index = 0; index < pending_.size(); ++index) {
    if (ids[index + 1].value == 0) (void)append(index);
  }
  pending_.clear();
  (void)tree->layout_root(LayoutInput{.constraints = {.max_size = viewport_}});
  return tree;
}

void WidgetTreeBuilder::install(const WindowRuntimeContext& context) {
  const ElementId* focus = focus_request_;
  focus_request_ = nullptr;
  // Every window runtime (root or record) routes, sets cursors and
  // publishes accessibility through its own element tree.
  context.set_element_tree(finish());
  for (const auto& [id, cursor] : cursors_) {
    context.set_element_cursor(id, cursor);
  }
  if (focus != nullptr && focus->value != 0) {
    context.request_keyboard_focus(*focus);
  }
}

} // namespace cgpui
