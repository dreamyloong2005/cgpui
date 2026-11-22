#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/runtime_handles.hpp"
#include "cgpui/ui/widget_tree.hpp"
#include "cgpui/ui/workbench_theme.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

class PaintList;
class WindowRuntime;
struct WindowRuntimeContext;
namespace workbench_detail {
class WidgetPopup;
}

using TooltipTargetId = std::uint64_t;

// pointer: below the pointer (above it when there is no room); anchor:
// below the target element (above it when there is no room).
enum class TooltipPosition {
  pointer,
  anchor,
};

struct TooltipOptions {
  std::uint64_t delay_ms = 500;
  std::uint64_t fade_ms = 120;
  TooltipPosition position = TooltipPosition::pointer;
  float max_width = 360.0F;
  float padding = 6.0F;
  float gap = 6.0F;
  float pointer_offset = 18.0F;
  // Show the bubble in its own popup window where the platform has popups
  // (it may then extend outside the window); the in-window overlay
  // otherwise.
  bool popup_window = true;
};

// One tooltip per window. The app registers hover targets (window rects and
// text) and forwards events; the tooltip appears after `delay_ms` over a
// target and hides on leave, press, scroll, Escape or window blur. The fade
// is skipped when the theme reports reduced motion.
class Tooltip {
 public:
  explicit Tooltip(TooltipOptions options = {});

  void set_target(TooltipTargetId id, Rect bounds, std::string_view text);
  void remove_target(TooltipTargetId id);
  void clear_targets();
  void hide();

  // Measures the active text (cached) and places the bubble inside
  // `viewport`; call it every render.
  void layout(Size viewport, DpiScale scale, const WorkbenchTheme& theme);
  EventResult handle_event(
      const PlatformEvent& event,
      const WindowRuntimeContext& context);
  [[nodiscard]] bool visible() const;
  [[nodiscard]] bool pending() const;
  [[nodiscard]] float opacity() const;
  [[nodiscard]] std::optional<TooltipTargetId> target() const;
  [[nodiscard]] Rect bounds() const;
  [[nodiscard]] bool flipped() const;

  void append_elements(
      WidgetTreeBuilder& builder,
      WidgetTreeBuilder::NodeHandle parent);
  void paint(PaintList& paint_list, const WorkbenchTheme& theme) const;

 private:
  struct Target {
    TooltipTargetId id = 0;
    Rect bounds;
    std::string text;
  };
  // Shared with timers and the fade animation, which hold weak references.
  struct Live {
    std::uint64_t generation = 0;
    bool pending = false;
    bool visible = false;
    float opacity = 0.0F;
    bool reduced_motion = false;
    std::uint64_t fade_ms = 0;
    AnimationHandle fade;
  };

  void paint_bubble(PaintList& paint_list, const WorkbenchTheme& theme) const;
  // Opens, moves or closes the popup window for the current state.
  void sync_popup(Rect anchor, Size size, const WorkbenchTheme& theme);
  [[nodiscard]] const Target* find(TooltipTargetId id) const;
  [[nodiscard]] std::optional<TooltipTargetId> target_at(Point point) const;
  void begin_hover(TooltipTargetId id, const WindowRuntimeContext& context);
  void hide_and_suppress();

  TooltipOptions options_;
  std::vector<Target> targets_;
  std::shared_ptr<Live> live_ = std::make_shared<Live>();
  std::optional<TooltipTargetId> active_;
  std::optional<TooltipTargetId> suppressed_;
  Point pointer_;
  Point anchor_pointer_;
  std::optional<TooltipTargetId> measured_target_;
  std::string measured_text_;
  float text_width_ = 0.0F;
  float font_size_ = 0.0F;
  DpiScale scale_;
  Rect bounds_;
  bool flipped_ = false;
  WindowRuntime* runtime_ = nullptr;  // set by the first hover
  std::shared_ptr<workbench_detail::WidgetPopup> popup_;
  std::shared_ptr<WorkbenchTheme> popup_theme_;
  bool in_popup_ = false;
};

} // namespace cgpui
