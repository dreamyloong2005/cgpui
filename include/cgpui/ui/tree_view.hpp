#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/uniform_list.hpp"
#include "cgpui/ui/widget_tree.hpp"
#include "cgpui/ui/workbench_theme.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cgpui {

class PaintList;
struct WindowRuntimeContext;

// App-chosen identity of an item; it must stay the same across updates so
// expansion, selection and focus follow the item.
using TreeItemId = std::uint64_t;

struct TreeItem {
  TreeItemId id = 0;
  std::string label;
  bool has_children = false;
};

enum class TreeViewEventKind {
  // An item without loaded children was expanded: the app answers with
  // set_children (now or later).
  children_requested,
  expanded,
  collapsed,
  selection_changed,
  activated,
  context_menu_requested,
};

struct TreeViewEvent {
  TreeViewEventKind kind = TreeViewEventKind::selection_changed;
  TreeItemId item = 0;
  Point position;
};

using TreeViewEventHandler = std::function<void(const TreeViewEvent&)>;

struct TreeViewOptions {
  float row_height = 22.0F;
  float indent = 16.0F;
  bool indent_guides = true;
  bool multi_select = true;
  std::size_t overscan = 4;
  std::uint64_t type_ahead_timeout_ms = 1000;
  std::string name = "Tree";
};

enum class TreeSelectMode {
  replace,
  toggle,
  extend,
};

// Virtualized hierarchical list. Only rows inside the viewport (plus
// `overscan`) get accessibility nodes and are painted; the flattened row list
// is rebuilt only when expansion or children change.
class TreeView {
 public:
  explicit TreeView(TreeViewOptions options = {});

  void set_roots(std::vector<TreeItem> roots);
  void set_children(TreeItemId parent, std::vector<TreeItem> children);
  [[nodiscard]] bool contains(TreeItemId item) const;
  [[nodiscard]] bool children_loaded(TreeItemId item) const;
  [[nodiscard]] std::optional<TreeItemId> parent(TreeItemId item) const;
  [[nodiscard]] std::size_t depth(TreeItemId item) const;
  void expand(TreeItemId item);
  void collapse(TreeItemId item);
  [[nodiscard]] bool expanded(TreeItemId item) const;
  void select(TreeItemId item, TreeSelectMode mode = TreeSelectMode::replace);
  void clear_selection();
  [[nodiscard]] bool selected(TreeItemId item) const;
  [[nodiscard]] std::vector<TreeItemId> selection() const;
  [[nodiscard]] std::optional<TreeItemId> focused_item() const;
  void on_event(TreeViewEventHandler handler);

  [[nodiscard]] std::size_t row_count() const;
  [[nodiscard]] std::optional<TreeItemId> item_at_row(std::size_t row) const;
  [[nodiscard]] std::optional<std::size_t> row_of(TreeItemId item) const;
  [[nodiscard]] std::optional<std::size_t> row_at(Point point) const;
  [[nodiscard]] Rect row_rect(std::size_t row) const;
  [[nodiscard]] UniformListVisibleRange visible_range() const;
  [[nodiscard]] float scroll_offset() const;
  void scroll_to(float offset);
  void scroll_into_view(TreeItemId item);
  [[nodiscard]] bool focused() const;
  [[nodiscard]] ElementId element_id() const;

  void layout(Rect bounds, DpiScale scale);
  void append_elements(
      WidgetTreeBuilder& builder,
      WidgetTreeBuilder::NodeHandle parent);
  EventResult handle_event(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
  void paint(PaintList& paint_list, const WorkbenchTheme& theme) const;

 private:
  struct Node {
    std::string label;
    TreeItemId parent = 0;
    bool has_parent = false;
    bool has_children = false;
    bool loaded = false;
    bool expanded = false;
    std::uint32_t depth = 0;
    std::vector<TreeItemId> children;
  };

  void erase_subtree(TreeItemId item);
  void update_children(
      std::vector<TreeItemId>& slots,
      std::vector<TreeItem> items,
      std::optional<TreeItemId> parent,
      std::uint32_t depth);
  void rebuild_rows() const;
  void ensure_rows() const;
  void emit(TreeViewEventKind kind, TreeItemId item, Point position = {});
  EventResult handle_item_action(TreeItemId item, AccessibilityActionKind kind);
  void focus_row(std::size_t row, TreeSelectMode mode);
  [[nodiscard]] float max_scroll() const;
  [[nodiscard]] std::size_t page_rows() const;
  EventResult handle_pointer(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
  EventResult handle_key(
      const KeyboardKey& key,
      const WindowRuntimeContext& context);
  EventResult handle_type_ahead(
      std::string_view text,
      const WindowRuntimeContext& context);

  TreeViewOptions options_;
  std::unordered_map<TreeItemId, Node> nodes_;
  std::vector<TreeItemId> roots_;
  mutable std::vector<TreeItemId> rows_;
  mutable std::unordered_map<TreeItemId, std::size_t> row_index_;
  mutable bool rows_dirty_ = true;
  std::unordered_set<TreeItemId> selected_;
  std::optional<TreeItemId> anchor_;
  std::optional<TreeItemId> focused_item_;
  std::optional<std::size_t> hovered_row_;
  TreeViewEventHandler handler_;
  Rect bounds_;
  DpiScale scale_;
  float scroll_ = 0.0F;
  ElementId tree_id_;
  bool focused_ = false;
  KeyboardModifiers held_;
  // Shared with the reset timer, which only holds a weak reference, so a
  // TreeView destroyed before the timer fires is never touched.
  struct TypeAhead {
    std::string text;
    std::uint64_t generation = 0;
  };
  std::shared_ptr<TypeAhead> type_ahead_ = std::make_shared<TypeAhead>();
};

} // namespace cgpui
