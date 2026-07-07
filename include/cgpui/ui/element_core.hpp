#pragma once

#include "cgpui/core/events.hpp"
#include "cgpui/ui/focus_metadata.hpp"
#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/style.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cgpui {

class PaintList;
class Element;
class ElementBuilder;

struct ElementId {
  std::uint64_t value = 0;

  friend bool operator==(ElementId, ElementId) = default;
};

struct ViewId {
  std::uint64_t value = 0;

  friend bool operator==(ViewId, ViewId) = default;
};

struct ElementKey {
  std::string value;

  friend bool operator==(const ElementKey&, const ElementKey&) = default;
};

enum class AccessibilityRole {
  generic,
  label,
  button,
  text,
  text_input,
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

  [[nodiscard]] const AccessibilityNode* node(ElementId element_id) const {
    const auto it = std::find_if(
        nodes.begin(),
        nodes.end(),
        [element_id](const AccessibilityNode& candidate) {
          return candidate.element_id == element_id;
        });
    return it == nodes.end() ? nullptr : &*it;
  }
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

  [[nodiscard]] ElementId id() const {
    return id_;
  }

  [[nodiscard]] const std::optional<ElementKey>& key() const {
    return key_;
  }

  [[nodiscard]] virtual LayoutOutput layout(LayoutInput input) const {
    const LayoutOutput output{
        .size = constrain_size({}, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    return output;
  }

  [[nodiscard]] std::optional<Rect> layout_bounds() const {
    return layout_bounds_;
  }

  [[nodiscard]] virtual ElementId hit_test(Point point) const {
    return layout_bounds_.has_value() && contains(*layout_bounds_, point) ? id_
                                                                         : ElementId{};
  }

  virtual void paint(PaintList& paint_list) const {
    (void)paint_list;
  }

  virtual void inherit_text_style(const Style& style) {
    (void)style;
  }

  [[nodiscard]] bool enabled() const {
    return enabled_;
  }

  void set_enabled(bool enabled) {
    enabled_ = enabled;
  }

  [[nodiscard]] FocusMetadata focus_metadata() const;
  void set_focus_metadata(FocusMetadata metadata);

  [[nodiscard]] float flex_grow() const {
    return flex_grow_;
  }

  void set_flex_grow(float value) {
    flex_grow_ = value;
  }

  [[nodiscard]] float flex_shrink() const {
    return flex_shrink_;
  }

  void set_flex_shrink(float value) {
    flex_shrink_ = value;
  }

  [[nodiscard]] Position position() const {
    return position_;
  }

  void set_position(Position value) {
    position_ = value;
  }

  [[nodiscard]] EdgeSizes inset() const {
    return inset_;
  }

  void set_inset(EdgeSizes value) {
    inset_ = value;
  }

  [[nodiscard]] virtual int z_index() const {
    return z_index_;
  }

  void set_z_index(int value) {
    z_index_ = value;
  }

  [[nodiscard]] virtual int layer() const {
    return layer_;
  }

  void set_layer(int value) {
    layer_ = value;
  }

  [[nodiscard]] int z_order() const {
    const int explicit_z_index = z_index();
    return explicit_z_index != 0 ? explicit_z_index : layer();
  }

  [[nodiscard]] virtual bool focusable() const {
    return false;
  }

  [[nodiscard]] virtual AccessibilityRole accessibility_role() const {
    return AccessibilityRole::generic;
  }

  [[nodiscard]] virtual std::string accessibility_name() const {
    return {};
  }

  [[nodiscard]] virtual std::string accessibility_text() const {
    return {};
  }

  virtual void focus(const ElementFocusContext& context) {
    (void)context;
  }

  virtual void on_mount(const ElementLifecycleContext& context) {
    (void)context;
  }

  virtual void on_update(const ElementLifecycleContext& context) {
    (void)context;
  }

  virtual void on_unmount(const ElementLifecycleContext& context) {
    (void)context;
  }

  [[nodiscard]] virtual EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) {
    (void)event;
    (void)context;
    return EventResult::unhandled();
  }

  void set_layout_bounds(Rect bounds) const {
    layout_bounds_ = bounds;
  }

  void assign_id(ElementId id) {
    id_ = id;
  }

  void set_key(std::optional<ElementKey> key) {
    key_ = std::move(key);
  }

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

[[nodiscard]] inline Point absolute_origin(Size parent_size, EdgeSizes inset) {
  (void)parent_size;
  return Point{.x = inset.left, .y = inset.top};
}

} // namespace cgpui
