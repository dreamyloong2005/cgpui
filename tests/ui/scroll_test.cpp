#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/uniform_list.hpp"

#include <type_traits>
#include <vector>

namespace {

bool same(float lhs, float rhs) {
  return lhs == rhs;
}

static_assert(std::is_same_v<cgpui::ScrollState, cgpui::ScrollModel>);

int test_scroll_model_defaults_to_zero_offset() {
  const cgpui::ScrollModel model;
  if (!same(model.offset().x, 0.0F) || !same(model.offset().y, 0.0F)) {
    return 1;
  }
  if (!same(model.viewport_size().width, 0.0F) ||
      !same(model.viewport_size().height, 0.0F)) {
    return 2;
  }
  return same(model.content_size().width, 0.0F) &&
                 same(model.content_size().height, 0.0F)
             ? 0
             : 3;
}

int test_scroll_model_clamps_offset_to_scrollable_extent() {
  cgpui::ScrollModel model;
  model.set_viewport_size(cgpui::Size{.width = 100.0F, .height = 50.0F});
  model.set_content_size(cgpui::Size{.width = 300.0F, .height = 140.0F});
  model.scroll_by(cgpui::Point{.x = 250.0F, .y = 120.0F});

  if (!same(model.offset().x, 200.0F) ||
      !same(model.offset().y, 90.0F)) {
    return 4;
  }

  model.scroll_by(cgpui::Point{.x = -500.0F, .y = -500.0F});
  return same(model.offset().x, 0.0F) && same(model.offset().y, 0.0F) ? 0 : 5;
}

int test_scroll_model_reclamps_when_sizes_change() {
  cgpui::ScrollModel model;
  model.set_viewport_size(cgpui::Size{.width = 100.0F, .height = 100.0F});
  model.set_content_size(cgpui::Size{.width = 180.0F, .height = 220.0F});
  model.set_offset(cgpui::Point{.x = 70.0F, .y = 110.0F});

  if (!same(model.offset().x, 70.0F) ||
      !same(model.offset().y, 110.0F)) {
    return 6;
  }

  model.set_content_size(cgpui::Size{.width = 120.0F, .height = 150.0F});
  if (!same(model.offset().x, 20.0F) ||
      !same(model.offset().y, 50.0F)) {
    return 7;
  }

  model.set_viewport_size(cgpui::Size{.width = 200.0F, .height = 200.0F});
  return same(model.offset().x, 0.0F) && same(model.offset().y, 0.0F) ? 0 : 8;
}

int test_scroll_model_reports_scrollable_axes() {
  cgpui::ScrollModel model;
  model.set_viewport_size(cgpui::Size{.width = 100.0F, .height = 100.0F});
  model.set_content_size(cgpui::Size{.width = 100.0F, .height = 160.0F});

  if (model.can_scroll_x() || !model.can_scroll_y()) {
    return 9;
  }

  model.set_content_size(cgpui::Size{.width = 160.0F, .height = 80.0F});
  return model.can_scroll_x() && !model.can_scroll_y() ? 0 : 10;
}

int test_scroll_state_alias_preserves_scroll_model_semantics() {
  cgpui::ScrollState state;
  state.set_viewport_size(cgpui::Size{.width = 40.0F, .height = 30.0F});
  state.set_content_size(cgpui::Size{.width = 90.0F, .height = 90.0F});
  state.scroll_by(cgpui::Point{.x = 80.0F, .y = 75.0F});

  return same(state.offset().x, 50.0F) && same(state.offset().y, 60.0F) &&
                 state.can_scroll_x() && state.can_scroll_y()
             ? 0
             : 11;
}

int test_uniform_list_visible_range_uses_stable_item_bounds() {
  const std::vector<cgpui::UniformListItemIdentity> items{
      {
          .index = 0,
          .key = cgpui::ElementKey{.value = "alpha"},
          .element_id = cgpui::ElementId{10},
          .content_bounds =
              cgpui::Rect{.origin = {.x = 0.0F, .y = 0.0F},
                           .size = {.width = 60.0F, .height = 10.0F}},
      },
      {
          .index = 1,
          .key = cgpui::ElementKey{.value = "beta"},
          .element_id = cgpui::ElementId{11},
          .content_bounds =
              cgpui::Rect{.origin = {.x = 0.0F, .y = 12.0F},
                           .size = {.width = 60.0F, .height = 10.0F}},
      },
      {
          .index = 2,
          .key = cgpui::ElementKey{.value = "gamma"},
          .element_id = cgpui::ElementId{12},
          .content_bounds =
              cgpui::Rect{.origin = {.x = 0.0F, .y = 24.0F},
                           .size = {.width = 60.0F, .height = 10.0F}},
      },
      {
          .index = 3,
          .key = cgpui::ElementKey{.value = "delta"},
          .element_id = cgpui::ElementId{13},
          .content_bounds =
              cgpui::Rect{.origin = {.x = 0.0F, .y = 36.0F},
                           .size = {.width = 60.0F, .height = 10.0F}},
      },
  };

  const cgpui::UniformListVisibleRange range =
      cgpui::calculate_uniform_list_visible_range(
          items,
          cgpui::Point{.x = 0.0F, .y = 13.0F},
          cgpui::Size{.width = 60.0F, .height = 24.0F});
  if (range.start_index != 1 || range.end_index != 4 || range.empty()) {
    return 12;
  }
  if (!range.contains(1) || !range.contains(3) || range.contains(0) ||
      range.contains(4)) {
    return 13;
  }

  const cgpui::UniformListVisibleRange empty_range =
      cgpui::calculate_uniform_list_visible_range(
          items,
          cgpui::Point{.x = 0.0F, .y = 100.0F},
          cgpui::Size{.width = 60.0F, .height = 24.0F});
  return empty_range.empty() && empty_range.start_index == 0 &&
                 empty_range.end_index == 0
             ? 0
             : 14;
}

} // namespace

int main() {
  if (const int result = test_scroll_model_defaults_to_zero_offset();
      result != 0) {
    return result;
  }
  if (const int result = test_scroll_model_clamps_offset_to_scrollable_extent();
      result != 0) {
    return result;
  }
  if (const int result = test_scroll_model_reclamps_when_sizes_change();
      result != 0) {
    return result;
  }
  if (const int result = test_scroll_model_reports_scrollable_axes();
      result != 0) {
    return result;
  }
  if (const int result = test_scroll_state_alias_preserves_scroll_model_semantics();
      result != 0) {
    return result;
  }
  if (const int result = test_uniform_list_visible_range_uses_stable_item_bounds();
      result != 0) {
    return result;
  }
  return 0;
}
