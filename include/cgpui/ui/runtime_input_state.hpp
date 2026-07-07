#pragma once

#include "cgpui/ui/runtime_diagnostics.hpp"
#include "cgpui/ui/view_handle.hpp"

#include <optional>
#include <variant>

namespace cgpui {

class PointerCaptureOwner {
 public:
  [[nodiscard]] static constexpr PointerCaptureOwner view(ViewId view_id) {
    return PointerCaptureOwner(view_id);
  }

  [[nodiscard]] static constexpr PointerCaptureOwner element(
      ElementId element_id) {
    return PointerCaptureOwner(element_id);
  }

  [[nodiscard]] constexpr bool is_view() const {
    return std::holds_alternative<ViewId>(owner_);
  }

  [[nodiscard]] constexpr bool is_element() const {
    return std::holds_alternative<ElementId>(owner_);
  }

  [[nodiscard]] constexpr const ViewId* view_id() const {
    return std::get_if<ViewId>(&owner_);
  }

  [[nodiscard]] constexpr const ElementId* element_id() const {
    return std::get_if<ElementId>(&owner_);
  }

  friend bool operator==(
      const PointerCaptureOwner&,
      const PointerCaptureOwner&) = default;

 private:
  using Owner = std::variant<ViewId, ElementId>;

  constexpr explicit PointerCaptureOwner(ViewId view_id) : owner_(view_id) {}
  constexpr explicit PointerCaptureOwner(ElementId element_id)
      : owner_(element_id) {}

  Owner owner_;
};

struct ViewInputState {
  bool focused = false;
  bool pointer_captured = false;
  std::optional<PointerCaptureOwner> pointer_capture_owner;
  bool keyboard_focused = false;
  std::optional<ViewId> keyboard_focus_owner;
  std::optional<ElementId> keyboard_focus_element_owner;
  std::optional<ElementId> hovered_element_id;
  std::optional<ElementId> active_element_id;
  std::optional<ElementId> pointer_down_element_id;
  std::optional<ElementId> clicked_element_id;
  std::optional<ElementId> dragging_element_id;
  bool dragging = false;
  CursorShape cursor_shape = CursorShape::default_arrow;
  Point pointer_position{};
};

class FocusHandle {
 public:
  constexpr FocusHandle() = default;
  explicit constexpr FocusHandle(ElementId id) : id_(id) {}

  [[nodiscard]] constexpr ElementId id() const { return id_; }
  [[nodiscard]] constexpr bool empty() const { return id_.value == 0; }

  void request(WindowRuntime& runtime) const;
  void request(const WindowRuntimeContext& context) const;
  void release(WindowRuntime& runtime) const;
  void release(const WindowRuntimeContext& context) const;

  [[nodiscard]] bool contains(const ViewInputState& input) const;
  [[nodiscard]] bool contains(const WindowRuntime& runtime) const;
  [[nodiscard]] bool contains(const WindowRuntimeContext& context) const;
  [[nodiscard]] bool focused(const ViewInputState& input) const;
  [[nodiscard]] bool focused(const WindowRuntime& runtime) const;
  [[nodiscard]] bool focused(const WindowRuntimeContext& context) const;

 private:
  ElementId id_{};
};

} // namespace cgpui
