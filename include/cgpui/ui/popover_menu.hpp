#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/anchored.hpp"
#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/widget_tree.hpp"
#include "cgpui/ui/workbench_theme.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cgpui {

class PaintList;
struct WindowRuntimeContext;

enum class MenuItemKind {
  action,
  separator,
  submenu,
};

struct MenuItem {
  MenuItemKind kind = MenuItemKind::action;
  std::string id;
  std::string label;
  std::string accelerator;
  bool enabled = true;
  std::vector<MenuItem> children;

  [[nodiscard]] static MenuItem action(
      std::string id,
      std::string label,
      std::string accelerator = {},
      bool enabled = true);
  [[nodiscard]] static MenuItem separator();
  [[nodiscard]] static MenuItem submenu(
      std::string label,
      std::vector<MenuItem> children,
      bool enabled = true);
};

enum class PopoverCloseReason {
  activated,
  escape,
  outside_click,
  focus_lost,
  app,
};

struct PopoverMenuOptions {
  float item_height = 26.0F;
  float separator_height = 9.0F;
  float padding = 4.0F;
  float item_padding = 10.0F;
  float min_width = 180.0F;
  float accelerator_gap = 32.0F;
  float submenu_arrow_width = 16.0F;
  float corner_radius = 8.0F;
  float margin = 4.0F;
  // C6 backdrop blur behind the surface (tint-only fallback when the
  // renderer has no blur).
  bool backdrop_blur = false;
  float blur_radius = 16.0F;
  std::string name = "Context menu";
};

using MenuActivateHandler = std::function<void(std::string_view item_id)>;
using PopoverCloseHandler = std::function<void(PopoverCloseReason reason)>;

// An in-window popover menu drawn above the content (GPUI `deferred` +
// `anchored`): a window-sized overlay node takes every pointer event while
// the menu is open, so outside clicks dismiss it. Opening moves keyboard
// focus into the menu; closing restores the previously focused element.
// Menus cannot extend outside the window.
class PopoverMenu {
 public:
  explicit PopoverMenu(PopoverMenuOptions options = {});

  // Context menu: top-left corner at `position`, flipped / shifted to fit.
  void open_at(
      Point position,
      std::vector<MenuItem> items,
      const WindowRuntimeContext& context);
  // Popover anchored to an element rectangle.
  void open_beside(
      Rect anchor,
      AnchoredSide side,
      std::vector<MenuItem> items,
      const WindowRuntimeContext& context);
  void close(PopoverCloseReason reason, const WindowRuntimeContext& context);
  [[nodiscard]] bool is_open() const;
  void on_activate(MenuActivateHandler handler);
  void on_close(PopoverCloseHandler handler);

  // Measures labels and accelerators with the theme font when a level opens
  // and positions every level inside `viewport`.
  void layout(Size viewport, DpiScale scale, const WorkbenchTheme& theme);
  EventResult handle_event(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
  void append_elements(
      WidgetTreeBuilder& builder,
      WidgetTreeBuilder::NodeHandle parent);
  void paint(PaintList& paint_list, const WorkbenchTheme& theme) const;

  [[nodiscard]] std::size_t level_count() const;
  [[nodiscard]] Rect level_bounds(std::size_t level) const;
  [[nodiscard]] Rect item_rect(std::size_t level, std::size_t index) const;
  [[nodiscard]] std::optional<std::size_t> highlighted(std::size_t level) const;
  [[nodiscard]] float label_column_width(std::size_t level) const;
  [[nodiscard]] float accelerator_column_x(std::size_t level) const;
  [[nodiscard]] float accelerator_width(std::size_t level, std::size_t index) const;
  [[nodiscard]] bool flipped_x(std::size_t level) const;
  [[nodiscard]] bool flipped_y(std::size_t level) const;
  [[nodiscard]] ElementId element_id() const;

 private:
  struct Level {
    const std::vector<MenuItem>* items = nullptr;
    std::optional<std::size_t> parent_index;
    std::vector<float> label_widths;
    std::vector<float> accelerator_widths;
    std::vector<float> item_y;
    float label_column = 0.0F;
    float accelerator_column = 0.0F;
    bool has_submenu = false;
    bool measured = false;
    Rect bounds;
    bool flipped_x = false;
    bool flipped_y = false;
    std::optional<std::size_t> highlighted;
  };

  void open(std::vector<MenuItem> items, const WindowRuntimeContext& context);
  void measure(Level& level);
  void place();
  void open_submenu(std::size_t level, std::size_t index);
  void truncate(std::size_t count);
  void move_highlight(int direction, bool to_edge);
  void activate(
      std::size_t level,
      std::size_t index,
      const WindowRuntimeContext& context);
  [[nodiscard]] bool selectable(const MenuItem& item) const;
  [[nodiscard]] std::optional<std::pair<std::size_t, std::size_t>> item_at(
      Point point) const;
  EventResult handle_key(
      const KeyboardKey& key,
      const WindowRuntimeContext& context);
  EventResult handle_pointer(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);

  PopoverMenuOptions options_;
  std::vector<MenuItem> items_;
  std::vector<Level> levels_;
  MenuActivateHandler activate_handler_;
  PopoverCloseHandler close_handler_;
  bool open_ = false;
  bool anchored_to_rect_ = false;
  Point position_;
  Rect anchor_;
  AnchoredSide side_ = AnchoredSide::below;
  Size viewport_;
  DpiScale scale_;
  FontDescriptor font_;
  float font_size_ = 13.0F;
  ElementId menu_id_;
  std::optional<ElementId> restore_focus_;
  bool focus_pending_ = false;
  bool pressed_inside_ = false;
};

} // namespace cgpui
