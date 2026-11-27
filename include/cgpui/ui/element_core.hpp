#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/accessibility.hpp"
#include "cgpui/ui/element_ids.hpp"
#include "cgpui/ui/focus_metadata.hpp"
#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/style.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

class PaintList;
class Element;
class ElementBuilder;

enum class AccessibilityRole {
  generic,
  label,
  button,
  text,
  text_input,
  image,
  checkbox,
  radio,
  switch_control,
  slider,
  list_item,
  menu_item,
  tab_list,
  tab,
  tree,
  tree_item,
  menu,
  separator,
  tooltip,
};

struct AccessibilitySnapshotOptions {
  std::optional<ElementId> focused_element_id;
};

struct AccessibilityNode {
  ElementId element_id;
  std::optional<ElementId> parent_element_id;
  AccessibilityRole role = AccessibilityRole::generic;
  std::string name;
  std::string text;
  std::string value;
  AccessibilityPatternState patterns;
  bool enabled = true;
  bool focusable = false;
  bool focused = false;
  std::optional<int> tab_index;
  FocusRingVisibility focus_ring = FocusRingVisibility::automatic;
  std::optional<Rect> bounds;
  std::vector<ElementId> children;
};

struct AccessibilityTreeSnapshot {
  ElementId root_element_id;
  std::vector<AccessibilityNode> nodes;

  [[nodiscard]] const AccessibilityNode* node(ElementId element_id) const;
};

struct EventResult {
  bool consumed = false;
  bool cancelled = false;

  [[nodiscard]] static constexpr EventResult unhandled() {
    return {};
  }

  [[nodiscard]] static constexpr EventResult consumed_event() {
    return {.consumed = true};
  }

  [[nodiscard]] static constexpr EventResult cancelled_event() {
    return {.consumed = true, .cancelled = true};
  }
};

enum class ElementGestureKind {
  none,
  click,
};

struct ElementEventContext {
  ElementId target_element_id;
  std::function<EventResult(std::string_view)> dispatch_action;
  ElementGestureKind gesture = ElementGestureKind::none;
};

struct ElementFocusContext {
  ElementId element_id;
};

struct ElementLifecycleContext {
  ElementId element_id;
  std::optional<ElementId> parent_element_id;
};

using ClickHandler = std::function<EventResult(const ElementEventContext&)>;
using PointerButtonHandler =
    std::function<EventResult(const PointerButton&, const ElementEventContext&)>;
using PointerMoveHandler =
    std::function<EventResult(const PointerMoved&, const ElementEventContext&)>;
using KeyHandler =
    std::function<EventResult(const KeyboardKey&, const ElementEventContext&)>;

using AnyElement = std::unique_ptr<Element>;

class Element {
 public:
  virtual ~Element() = default;

  [[nodiscard]] ElementId id() const;

  [[nodiscard]] const std::optional<ElementKey>& key() const;

  [[nodiscard]] virtual LayoutOutput layout(LayoutInput input) const;

  [[nodiscard]] std::optional<Rect> layout_bounds() const;

  [[nodiscard]] virtual ElementId hit_test(Point point) const;

  virtual void paint(PaintList& paint_list) const;

  virtual void inherit_text_style(const Style& style);

  [[nodiscard]] bool enabled() const;

  void set_enabled(bool enabled);

  [[nodiscard]] FocusMetadata focus_metadata() const;
  void set_focus_metadata(FocusMetadata metadata);

  [[nodiscard]] float flex_grow() const;

  void set_flex_grow(float value);

  [[nodiscard]] float flex_shrink() const;

  void set_flex_shrink(float value);

  [[nodiscard]] Position position() const;

  void set_position(Position value);

  [[nodiscard]] EdgeSizes inset() const;

  void set_inset(EdgeSizes value);

  [[nodiscard]] virtual int z_index() const;

  void set_z_index(int value);

  [[nodiscard]] virtual int layer() const;

  void set_layer(int value);

  [[nodiscard]] int z_order() const;

  [[nodiscard]] virtual bool focusable() const;

  [[nodiscard]] virtual AccessibilityRole accessibility_role() const;

  [[nodiscard]] virtual std::string accessibility_name() const;

  [[nodiscard]] virtual std::string accessibility_text() const;

  [[nodiscard]] virtual std::string accessibility_value() const;

  [[nodiscard]] virtual AccessibilityPatternState accessibility_patterns()
      const;

  [[nodiscard]] virtual EventResult handle_accessibility_action(
      const AccessibilityActionRequested& action,
      const ElementEventContext& context);

  virtual void focus(const ElementFocusContext& context);

  virtual void on_mount(const ElementLifecycleContext& context);

  virtual void on_update(const ElementLifecycleContext& context);

  virtual void on_unmount(const ElementLifecycleContext& context);

  [[nodiscard]] virtual EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context);

  void set_layout_bounds(Rect bounds) const;

  // Layout bounds are window coordinates at every depth (GPUI model). An
  // element lays its children out around its own bounds as set by layout();
  // the parent then places it, which moves the element and every
  // descendant by the same offset.
  void place_layout(Rect bounds) const;
  void offset_layout(Point delta) const;

  void assign_id(ElementId id);

  void set_key(std::optional<ElementKey> key);

 protected:
  // Containers forward to offset_layout() of each child they own.
  virtual void offset_layout_children(Point delta) const;

 private:
  ElementId id_;
  std::optional<ElementKey> key_;
  FocusMetadata focus_metadata_;
  bool enabled_ = true;
  float flex_grow_ = 0.0F;
  float flex_shrink_ = 0.0F;
  Position position_ = Position::relative;
  EdgeSizes inset_;
  int z_index_ = 0;
  int layer_ = 0;
  mutable std::optional<Rect> layout_bounds_;
};

[[nodiscard]] Point absolute_origin(Size parent_size, EdgeSizes inset);

} // namespace cgpui
