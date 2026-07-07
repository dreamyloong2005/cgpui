#pragma once

#include "cgpui/ui/element_containers.hpp"
#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/uniform_list.hpp"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace cgpui {

class ScrollElement : public Element {
 public:
  ScrollElement(ScrollState& state, std::unique_ptr<Element> child)
      : state_(&state),
        child_(std::move(child)) {}

  [[nodiscard]] ScrollState* state() {
    return state_;
  }

  [[nodiscard]] const ScrollState* state() const {
    return state_;
  }

  [[nodiscard]] Element* child() {
    return child_.get();
  }

  [[nodiscard]] const Element* child() const {
    return child_.get();
  }

  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override {
    if (child_ == nullptr) {
      const LayoutOutput output = Element::layout(input);
      if (state_ != nullptr) {
        state_->set_viewport_size(output.size);
        state_->set_content_size(output.size);
      }
      return output;
    }

    const LayoutOutput child_output =
        child_->layout(LayoutInput{.scale = input.scale});
    child_->set_layout_bounds(Rect{
        .origin = child_output.origin,
        .size = child_output.size,
    });
    const LayoutOutput output{
        .size = constrain_size(child_output.size, input.constraints),
    };
    set_layout_bounds(Rect{
        .origin = output.origin,
        .size = output.size,
    });
    if (state_ != nullptr) {
      state_->set_viewport_size(output.size);
      state_->set_content_size(child_output.size);
    }
    return output;
  }

  [[nodiscard]] ElementId hit_test(Point point) const override {
    return Element::hit_test(point);
  }

  void paint(PaintList& paint_list) const override {
    if (child_) {
      child_->paint(paint_list);
    }
  }

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override {
    return child_ == nullptr || !child_->enabled()
               ? EventResult::unhandled()
               : child_->handle_event(event, context);
  }

 private:
  ScrollState* state_ = nullptr;
  std::unique_ptr<Element> child_;
};

class ScrollableListElement : public Element {
 public:
  ScrollableListElement(
      ScrollState& state,
      Style style,
      std::vector<AnyElement> items,
      float gap = 0.0F)
      : state_(&state),
        style_(std::move(style)) {
    content_.set_gap(gap);
    for (auto& item : items) {
      content_.append_child(std::move(item));
    }
  }

  [[nodiscard]] ScrollState* state() {
    return state_;
  }

  [[nodiscard]] const ScrollState* state() const {
    return state_;
  }

  [[nodiscard]] VerticalStackElement& content() {
    return content_;
  }

  [[nodiscard]] const VerticalStackElement& content() const {
    return content_;
  }

  [[nodiscard]] std::size_t item_count() const {
    return content_.children().size();
  }

  [[nodiscard]] const Style& style() const {
    return style_;
  }

  [[nodiscard]] const UniformListLayoutSnapshot& layout_snapshot() const;
  [[nodiscard]] const UniformListItemMeasurementCache& measurement_cache() const;
  [[nodiscard]] LayoutOutput layout(LayoutInput input) const override;

  [[nodiscard]] ElementId hit_test(Point point) const override {
    return Element::hit_test(point);
  }

  void paint(PaintList& paint_list) const override;

  [[nodiscard]] EventResult handle_event(
      const PlatformEvent& event,
      const ElementEventContext& context) override {
    if (!enabled()) {
      return EventResult::unhandled();
    }
    return content_.handle_event(event, context);
  }

  [[nodiscard]] int z_index() const override {
    return style_.z_index;
  }

  [[nodiscard]] int layer() const override {
    return style_.layer;
  }

 private:
  ScrollState* state_ = nullptr;
  Style style_;
  mutable VerticalStackElement content_;
  mutable UniformListLayoutSnapshot layout_snapshot_;
  mutable UniformListItemMeasurementCache measurement_cache_;
};

} // namespace cgpui
