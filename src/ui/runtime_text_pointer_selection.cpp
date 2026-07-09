#include "ui_internal.hpp"

namespace cgpui {

bool WindowRuntime::apply_text_pointer_selection(
    const PlatformEvent& event,
    const EventRoute& route) {
  if (const auto* button = std::get_if<PointerButton>(&event);
      button != nullptr && button->button == MouseButton::left) {
    if (button->pressed) {
      text_pointer_selection_drag_.reset();
      if (!route.target_element_id.has_value()) {
        return false;
      }

      TextInputElement* input = routed_text_input(*route.target_element_id);
      if (input == nullptr || !input->enabled() || input->model() == nullptr) {
        return false;
      }

      const std::optional<std::size_t> offset =
          text_offset_for_point(*input, button->position);
      if (!offset.has_value()) {
        return false;
      }

      const TextSelectionGranularity granularity =
          text_selection_granularity_for_click_count(button->click_count);
      if (granularity == TextSelectionGranularity::word) {
        const TextSelectionRange selection =
            input->model()->word_selection_range_at(*offset);
        input->model()->set_selection(selection.start, selection.end);
        return false;
      }
      if (granularity == TextSelectionGranularity::line) {
        const TextSelectionRange selection =
            input->model()->line_selection_range_at(*offset);
        input->model()->set_selection(selection.start, selection.end);
        return false;
      }

      const TextSelectionDrag selection =
          text_selection_drag_from_offsets(*offset, *offset);
      input->model()->set_selection(
          selection.anchor_offset,
          selection.head_offset);
      text_pointer_selection_drag_ = TextPointerSelectionDrag{
          .element_id = *route.target_element_id,
          .anchor_offset = selection.anchor_offset,
      };
      return false;
    }

    if (!text_pointer_selection_drag_.has_value()) {
      return false;
    }

    const TextPointerSelectionDrag drag = *text_pointer_selection_drag_;
    text_pointer_selection_drag_.reset();
    TextInputElement* input = routed_text_input(drag.element_id);
    if (input == nullptr || input->model() == nullptr) {
      return true;
    }

    const std::optional<std::size_t> offset =
        text_offset_for_point(*input, button->position);
    if (offset.has_value()) {
      const TextSelectionDrag selection =
          text_selection_drag_from_offsets(drag.anchor_offset, *offset);
      input->model()->set_selection(
          selection.anchor_offset,
          selection.head_offset);
    }
    return true;
  }

  if (const auto* moved = std::get_if<PointerMoved>(&event);
      moved != nullptr && text_pointer_selection_drag_.has_value()) {
    const TextPointerSelectionDrag drag = *text_pointer_selection_drag_;
    TextInputElement* input = routed_text_input(drag.element_id);
    if (input == nullptr || input->model() == nullptr) {
      text_pointer_selection_drag_.reset();
      return false;
    }

    const std::optional<std::size_t> offset =
        text_offset_for_point(*input, moved->position);
    if (offset.has_value()) {
      const TextSelectionDrag selection =
          text_selection_drag_from_offsets(drag.anchor_offset, *offset);
      input->model()->set_selection(
          selection.anchor_offset,
          selection.head_offset);
    }
    return true;
  }

  return false;
}

std::optional<std::size_t> WindowRuntime::text_offset_for_point(
    const TextElement& element,
    Point point) const {
  const std::optional<Rect> bounds = element.layout_bounds();
  if (!bounds.has_value()) {
    return {};
  }

  const TextMeasurement measurement =
      measure_text(element.text(), element.font(), element.font_size(), scale_);
  return hit_test_text_position(measurement, *bounds, point).byte_offset;
}

} // namespace cgpui
