#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/widget_tree.hpp"
#include "cgpui/ui/workbench_theme.hpp"

#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace cgpui {

class PaintList;
struct WindowRuntimeContext;

// horizontal: panes left to right, vertical dividers; vertical: top to
// bottom, horizontal dividers.
enum class SplitterAxis {
  horizontal,
  vertical,
};

struct SplitterPaneOptions {
  float default_fraction = 1.0F;
  float min_size = 0.0F;
  float max_size = std::numeric_limits<float>::infinity();
  bool collapsible = false;
};

struct SplitterOptions {
  SplitterAxis axis = SplitterAxis::horizontal;
  float divider_thickness = 1.0F;
  float hit_thickness = 8.0F;
  float keyboard_step = 16.0F;
  float keyboard_large_step = 64.0F;
  std::string name = "Splitter";
};

enum class SplitterChangeSource {
  pointer,
  keyboard,
  restore,
  collapse,
  app,
};

// Reported after every change; `finished` is false while a drag is still
// moving, so an app can persist only finished ratios.
struct SplitterChange {
  std::span<const float> fractions;
  std::size_t divider = 0;
  SplitterChangeSource source = SplitterChangeSource::app;
  bool finished = true;
};

using SplitterChangeHandler = std::function<void(const SplitterChange&)>;

// Resizable panes separated by dividers. The app owns the Splitter and the
// persisted ratios (`fractions`, one per pane, summing to 1); the widget
// reports changes and never stores anything itself.
class Splitter {
 public:
  Splitter(SplitterOptions options, std::vector<SplitterPaneOptions> panes);

  [[nodiscard]] const SplitterOptions& options() const;
  [[nodiscard]] std::size_t pane_count() const;
  [[nodiscard]] std::span<const float> fractions() const;
  void set_fractions(std::span<const float> fractions);
  void restore_default_fractions();
  [[nodiscard]] bool collapsed(std::size_t pane) const;
  void set_collapsed(std::size_t pane, bool collapsed);
  void on_change(SplitterChangeHandler handler);

  // Positions panes and dividers inside `bounds` (window coordinates);
  // pane edges land on device pixels for `scale`.
  void layout(Rect bounds, DpiScale scale);
  [[nodiscard]] Rect bounds() const;
  [[nodiscard]] Rect pane_rect(std::size_t pane) const;
  [[nodiscard]] Rect divider_rect(std::size_t divider) const;
  [[nodiscard]] Rect divider_hit_rect(std::size_t divider) const;
  [[nodiscard]] std::optional<std::size_t> divider_at(Point point) const;
  [[nodiscard]] std::optional<std::size_t> dragging_divider() const;
  [[nodiscard]] std::optional<std::size_t> focused_divider() const;
  [[nodiscard]] ElementId divider_element_id(std::size_t divider) const;

  // Moves divider `divider` by `delta` logical pixels along the axis,
  // honoring min / max sizes and collapsing collapsible panes.
  bool move_divider(
      std::size_t divider,
      float delta,
      SplitterChangeSource source,
      bool finished = true);

  void append_elements(
      WidgetTreeBuilder& builder,
      WidgetTreeBuilder::NodeHandle parent);
  EventResult handle_event(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
  void paint(PaintList& paint_list, const WorkbenchTheme& theme) const;

 private:
  [[nodiscard]] float main_extent() const;
  [[nodiscard]] float available() const;
  [[nodiscard]] float main_of(Point point) const;
  void compute_sizes();
  void store_fractions_from_sizes();
  void notify(std::size_t divider, SplitterChangeSource source, bool finished);
  EventResult handle_pointer(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
  EventResult handle_key(
      const KeyboardKey& key,
      const WindowRuntimeContext& context);

  SplitterOptions options_;
  std::vector<SplitterPaneOptions> panes_;
  std::vector<float> fractions_;
  std::vector<bool> collapsed_;
  std::vector<float> sizes_;
  std::vector<bool> scratch_fixed_;
  std::vector<Rect> pane_rects_;
  std::vector<Rect> divider_rects_;
  std::vector<ElementId> divider_ids_;
  ElementId shield_id_;
  SplitterChangeHandler change_handler_;
  Rect bounds_;
  DpiScale scale_;
  std::optional<std::size_t> dragging_;
  std::optional<std::size_t> hovered_;
  std::optional<std::size_t> focused_;
  float drag_grab_offset_ = 0.0F;
};

} // namespace cgpui
