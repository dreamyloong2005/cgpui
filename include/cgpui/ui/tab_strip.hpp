#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/widget_tree.hpp"
#include "cgpui/ui/workbench_theme.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace cgpui {

class PaintList;
struct WindowRuntimeContext;

using TabId = std::uint64_t;

struct TabItem {
  TabId id = 0;
  std::string title;
  bool dirty = false;
  bool closable = true;
};

enum class TabStripEventKind {
  select,
  close,
  reorder,
};

// select / close carry the tab; reorder carries the tab and its move from
// `from_index` to `to_index`. The strip never changes its own tab list: the
// app applies the change and calls set_tabs / set_selected.
struct TabStripEvent {
  TabStripEventKind kind = TabStripEventKind::select;
  TabId tab = 0;
  std::size_t from_index = 0;
  std::size_t to_index = 0;
};

using TabStripEventHandler = std::function<void(const TabStripEvent&)>;

struct TabStripOptions {
  float height = 34.0F;
  float min_tab_width = 72.0F;
  float max_tab_width = 240.0F;
  float padding = 12.0F;
  float close_size = 18.0F;
  float drag_threshold = 5.0F;
  std::string name = "Editor tabs";
};

// A horizontal tab strip. Overflow scrolls the strip horizontally (wheel,
// and the selected tab is scrolled into view); there is no overflow menu.
class TabStrip {
 public:
  explicit TabStrip(TabStripOptions options = {});

  void set_tabs(std::vector<TabItem> tabs);
  [[nodiscard]] std::span<const TabItem> tabs() const;
  void set_selected(std::optional<TabId> tab);
  [[nodiscard]] std::optional<TabId> selected() const;
  [[nodiscard]] std::optional<std::size_t> index_of(TabId tab) const;
  void on_event(TabStripEventHandler handler);

  // Measures titles with the theme font (cached until titles, font or scale
  // change) and positions tabs inside `bounds`.
  void layout(Rect bounds, DpiScale scale, const WorkbenchTheme& theme);
  [[nodiscard]] Rect bounds() const;
  [[nodiscard]] Rect tab_rect(std::size_t index) const;
  [[nodiscard]] Rect close_rect(std::size_t index) const;
  [[nodiscard]] float title_width(std::size_t index) const;
  [[nodiscard]] float scroll_offset() const;
  [[nodiscard]] float content_width() const;
  void scroll_by(float delta);
  void scroll_into_view(std::size_t index);
  [[nodiscard]] std::optional<std::size_t> tab_at(Point point) const;
  [[nodiscard]] bool dragging() const;
  [[nodiscard]] std::optional<std::size_t> drop_index() const;
  [[nodiscard]] bool focused() const;
  [[nodiscard]] ElementId element_id() const;

  void append_elements(
      WidgetTreeBuilder& builder,
      WidgetTreeBuilder::NodeHandle parent);
  EventResult handle_event(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
  void paint(PaintList& paint_list, const WorkbenchTheme& theme) const;

 private:
  void emit(TabStripEventKind kind, std::size_t index, std::size_t to = 0);
  void place_tabs();
  [[nodiscard]] float clamp_scroll(float offset) const;
  [[nodiscard]] std::size_t drop_index_for(float x) const;
  EventResult handle_pointer(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
  EventResult handle_key(
      const KeyboardKey& key,
      const WindowRuntimeContext& context);

  TabStripOptions options_;
  std::vector<TabItem> tabs_;
  std::vector<float> title_widths_;
  std::vector<float> tab_x_;
  std::vector<float> tab_widths_;
  std::optional<TabId> selected_;
  TabStripEventHandler handler_;
  Rect bounds_;
  DpiScale scale_;
  float measured_font_size_ = 0.0F;
  float measured_scale_ = 0.0F;
  FontDescriptor font_;
  bool measure_dirty_ = true;
  bool reveal_pending_ = false;
  float scroll_ = 0.0F;
  float content_width_ = 0.0F;
  ElementId strip_id_;
  bool focused_ = false;
  std::optional<std::size_t> hovered_;
  bool hover_close_ = false;
  std::optional<std::size_t> pressed_;
  bool pressed_close_ = false;
  std::optional<std::size_t> middle_pressed_;
  bool dragging_ = false;
  float press_x_ = 0.0F;
  float drag_x_ = 0.0F;
};

} // namespace cgpui
