#include "cgpui/ui/tree_view.hpp"

#include <utility>

namespace cgpui {

TreeView::TreeView(TreeViewOptions options) : options_(std::move(options)) {}

void TreeView::set_roots(std::vector<TreeItem> roots) {
  update_children(roots_, std::move(roots), std::nullopt, 0);
}

void TreeView::set_children(TreeItemId parent, std::vector<TreeItem> children) {
  const auto found = nodes_.find(parent);
  if (found == nodes_.end()) return;
  found->second.loaded = true;
  found->second.has_children = !children.empty();
  const std::uint32_t depth = found->second.depth + 1;
  // update_children inserts into nodes_, so work on a moved-out slot list.
  std::vector<TreeItemId> slots = std::move(found->second.children);
  update_children(slots, std::move(children), parent, depth);
  nodes_.at(parent).children = std::move(slots);
}

// Keeps nodes whose id survives (with their expansion, loaded children and
// selection), inserts new ids and erases the subtrees of removed ids.
void TreeView::update_children(
    std::vector<TreeItemId>& slots,
    std::vector<TreeItem> items,
    std::optional<TreeItemId> parent,
    std::uint32_t depth) {
  std::unordered_set<TreeItemId> incoming;
  incoming.reserve(items.size());
  for (const TreeItem& item : items) incoming.insert(item.id);
  for (const TreeItemId old : slots) {
    if (!incoming.contains(old)) erase_subtree(old);
  }
  slots.clear();
  slots.reserve(items.size());
  nodes_.reserve(nodes_.size() + items.size());
  for (TreeItem& item : items) {
    Node& node = nodes_[item.id];
    node.label = std::move(item.label);
    node.has_children = item.has_children || !node.children.empty();
    node.parent = parent.value_or(0);
    node.has_parent = parent.has_value();
    node.depth = depth;
    slots.push_back(item.id);
  }
  rows_dirty_ = true;
}

void TreeView::erase_subtree(TreeItemId item) {
  std::vector<TreeItemId> stack{item};
  while (!stack.empty()) {
    const TreeItemId current = stack.back();
    stack.pop_back();
    const auto found = nodes_.find(current);
    if (found == nodes_.end()) continue;
    stack.insert(stack.end(), found->second.children.begin(),
                 found->second.children.end());
    nodes_.erase(found);
    selected_.erase(current);
    if (focused_item_ == current) focused_item_.reset();
    if (anchor_ == current) anchor_.reset();
  }
  rows_dirty_ = true;
}

bool TreeView::contains(TreeItemId item) const {
  return nodes_.contains(item);
}

bool TreeView::children_loaded(TreeItemId item) const {
  const auto found = nodes_.find(item);
  return found != nodes_.end() && found->second.loaded;
}

std::optional<TreeItemId> TreeView::parent(TreeItemId item) const {
  const auto found = nodes_.find(item);
  if (found == nodes_.end() || !found->second.has_parent) return std::nullopt;
  return found->second.parent;
}

std::size_t TreeView::depth(TreeItemId item) const {
  const auto found = nodes_.find(item);
  return found == nodes_.end() ? 0 : found->second.depth;
}

void TreeView::expand(TreeItemId item) {
  const auto found = nodes_.find(item);
  if (found == nodes_.end() || !found->second.has_children ||
      found->second.expanded) {
    return;
  }
  found->second.expanded = true;
  rows_dirty_ = true;
  const bool request = !found->second.loaded;
  emit(TreeViewEventKind::expanded, item);
  if (request) emit(TreeViewEventKind::children_requested, item);
}

void TreeView::collapse(TreeItemId item) {
  const auto found = nodes_.find(item);
  if (found == nodes_.end() || !found->second.expanded) return;
  found->second.expanded = false;
  rows_dirty_ = true;
  // Focus inside the collapsed subtree moves to the collapsed item.
  for (std::optional<TreeItemId> at = focused_item_; at.has_value();
       at = parent(*at)) {
    if (*at == item && focused_item_ != item) {
      focused_item_ = item;
      break;
    }
  }
  emit(TreeViewEventKind::collapsed, item);
}

bool TreeView::expanded(TreeItemId item) const {
  const auto found = nodes_.find(item);
  return found != nodes_.end() && found->second.expanded;
}

void TreeView::rebuild_rows() const {
  rows_.clear();
  row_index_.clear();
  std::vector<TreeItemId> stack(roots_.rbegin(), roots_.rend());
  while (!stack.empty()) {
    const TreeItemId current = stack.back();
    stack.pop_back();
    const auto found = nodes_.find(current);
    if (found == nodes_.end()) continue;
    row_index_.emplace(current, rows_.size());
    rows_.push_back(current);
    const Node& node = found->second;
    if (node.expanded && node.loaded) {
      stack.insert(stack.end(), node.children.rbegin(), node.children.rend());
    }
  }
  rows_dirty_ = false;
}

void TreeView::ensure_rows() const {
  if (rows_dirty_) rebuild_rows();
}

std::size_t TreeView::row_count() const {
  ensure_rows();
  return rows_.size();
}

std::optional<TreeItemId> TreeView::item_at_row(std::size_t row) const {
  ensure_rows();
  if (row >= rows_.size()) return std::nullopt;
  return rows_[row];
}

std::optional<std::size_t> TreeView::row_of(TreeItemId item) const {
  ensure_rows();
  const auto found = row_index_.find(item);
  if (found == row_index_.end()) return std::nullopt;
  return found->second;
}

void TreeView::select(TreeItemId item, TreeSelectMode mode) {
  if (!nodes_.contains(item)) return;
  if (!options_.multi_select) mode = TreeSelectMode::replace;
  if (mode == TreeSelectMode::toggle) {
    if (!selected_.erase(item)) selected_.insert(item);
    anchor_ = item;
  } else if (mode == TreeSelectMode::extend && anchor_.has_value() &&
             row_of(*anchor_).has_value() && row_of(item).has_value()) {
    std::size_t first = *row_of(*anchor_);
    std::size_t last = *row_of(item);
    if (first > last) std::swap(first, last);
    selected_.clear();
    for (std::size_t row = first; row <= last; ++row) selected_.insert(rows_[row]);
  } else {
    selected_.clear();
    selected_.insert(item);
    anchor_ = item;
  }
  focused_item_ = item;
  emit(TreeViewEventKind::selection_changed, item);
}

void TreeView::clear_selection() {
  selected_.clear();
  anchor_.reset();
}

bool TreeView::selected(TreeItemId item) const {
  return selected_.contains(item);
}

// Selected items in row order (collapsed-away selections last).
std::vector<TreeItemId> TreeView::selection() const {
  ensure_rows();
  std::vector<TreeItemId> result;
  result.reserve(selected_.size());
  for (const TreeItemId row : rows_) {
    if (result.size() == selected_.size()) break;
    if (selected_.contains(row)) result.push_back(row);
  }
  for (const TreeItemId item : selected_) {
    if (!row_index_.contains(item)) result.push_back(item);
  }
  return result;
}

std::optional<TreeItemId> TreeView::focused_item() const {
  return focused_item_;
}

void TreeView::on_event(TreeViewEventHandler handler) {
  handler_ = std::move(handler);
}

// Accessibility actions on a row (UIA ExpandCollapse / SelectionItem).
EventResult TreeView::handle_item_action(TreeItemId item, AccessibilityActionKind kind) {
  if (!nodes_.contains(item)) return EventResult::unhandled();
  const bool is_selected = selected(item);
  if (kind == AccessibilityActionKind::expand) {
    expand(item);
  } else if (kind == AccessibilityActionKind::collapse) {
    collapse(item);
  } else if (kind == AccessibilityActionKind::select) {
    select(item, TreeSelectMode::replace);
  } else if (kind == AccessibilityActionKind::add_to_selection && !is_selected) {
    select(item, TreeSelectMode::toggle);
  } else if (kind == AccessibilityActionKind::remove_from_selection && is_selected) {
    select(item, TreeSelectMode::toggle);
  } else if (kind != AccessibilityActionKind::add_to_selection &&
             kind != AccessibilityActionKind::remove_from_selection) {
    return EventResult::unhandled();
  }
  return EventResult::consumed_event();
}

void TreeView::emit(TreeViewEventKind kind, TreeItemId item, Point position) {
  if (handler_) handler_(TreeViewEvent{.kind = kind, .item = item, .position = position});
}

} // namespace cgpui
