#include "cgpui/ui/scroll.hpp"
#include "cgpui/ui/uniform_list.hpp"
#include "cgpui/ui/uniform_list_selection.hpp"

#include <optional>
#include <span>
#include <string>
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

int test_scroll_model_scrolls_rect_into_view() {
  cgpui::ScrollModel model;
  model.set_viewport_size(cgpui::Size{.width = 100.0F, .height = 50.0F});
  model.set_content_size(cgpui::Size{.width = 300.0F, .height = 200.0F});

  model.scroll_rect_into_view(cgpui::Rect{
      .origin = {.x = 10.0F, .y = 8.0F},
      .size = {.width = 20.0F, .height = 15.0F},
  });
  if (!same(model.offset().x, 0.0F) || !same(model.offset().y, 0.0F)) {
    return 31;
  }

  model.scroll_rect_into_view(cgpui::Rect{
      .origin = {.x = 80.0F, .y = 45.0F},
      .size = {.width = 40.0F, .height = 20.0F},
  });
  if (!same(model.offset().x, 20.0F) || !same(model.offset().y, 15.0F)) {
    return 32;
  }

  model.scroll_rect_into_view(cgpui::Rect{
      .origin = {.x = 15.0F, .y = 5.0F},
      .size = {.width = 10.0F, .height = 5.0F},
  });
  if (!same(model.offset().x, 15.0F) || !same(model.offset().y, 5.0F)) {
    return 33;
  }

  model.scroll_rect_into_view(cgpui::Rect{
      .origin = {.x = 260.0F, .y = 180.0F},
      .size = {.width = 80.0F, .height = 40.0F},
  });
  return same(model.offset().x, 200.0F) && same(model.offset().y, 150.0F) ? 0
                                                                         : 34;
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

int test_uniform_list_scroll_anchor_preserves_keyed_item_position() {
  const std::vector<cgpui::UniformListItemIdentity> before{
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
  };

  const std::optional<cgpui::UniformListScrollAnchor> anchor =
      cgpui::capture_uniform_list_scroll_anchor(
          before,
          cgpui::Point{.x = 0.0F, .y = 13.0F});
  if (!anchor.has_value() || anchor->key.value != "beta" ||
      anchor->index != 1 || anchor->element_id != cgpui::ElementId{11} ||
      anchor->viewport_offset_y != -1.0F) {
    return 15;
  }

  const std::vector<cgpui::UniformListItemIdentity> after{
      {
          .index = 0,
          .key = cgpui::ElementKey{.value = "alpha"},
          .element_id = cgpui::ElementId{10},
          .content_bounds =
              cgpui::Rect{.origin = {.x = 0.0F, .y = 0.0F},
                           .size = {.width = 60.0F, .height = 22.0F}},
      },
      {
          .index = 1,
          .key = cgpui::ElementKey{.value = "beta"},
          .element_id = cgpui::ElementId{11},
          .content_bounds =
              cgpui::Rect{.origin = {.x = 0.0F, .y = 24.0F},
                           .size = {.width = 60.0F, .height = 10.0F}},
      },
      {
          .index = 2,
          .key = cgpui::ElementKey{.value = "gamma"},
          .element_id = cgpui::ElementId{12},
          .content_bounds =
              cgpui::Rect{.origin = {.x = 0.0F, .y = 36.0F},
                           .size = {.width = 60.0F, .height = 10.0F}},
      },
  };
  const cgpui::Point adjusted = cgpui::apply_uniform_list_scroll_anchor(
      after,
      *anchor,
      cgpui::Point{.x = 5.0F, .y = 13.0F});
  if (adjusted.x != 5.0F || adjusted.y != 25.0F) {
    return 16;
  }

  const cgpui::Point unchanged = cgpui::apply_uniform_list_scroll_anchor(
      std::span<const cgpui::UniformListItemIdentity>{},
      *anchor,
      cgpui::Point{.x = 5.0F, .y = 13.0F});
  return unchanged.x == 5.0F && unchanged.y == 13.0F ? 0 : 17;
}

int test_uniform_list_item_measurement_cache_reuses_keyed_sizes() {
  cgpui::UniformListItemMeasurementCache cache;
  const cgpui::UniformListItemIdentity alpha{
      .index = 0,
      .key = cgpui::ElementKey{.value = "alpha"},
      .element_id = cgpui::ElementId{10},
      .content_bounds =
          cgpui::Rect{.origin = {.x = 0.0F, .y = 0.0F},
                       .size = {.width = 60.0F, .height = 10.0F}},
  };
  const cgpui::UniformListItemMeasurementResult first = cache.measure(alpha);
  if (first.cache_hit || cache.entry_count() != 1 ||
      cache.lookup_count() != 1 || cache.miss_count() != 1 ||
      cache.hit_count() != 0 ||
      first.measurement.size.height != 10.0F) {
    return 18;
  }

  const cgpui::UniformListItemIdentity resized_alpha{
      .index = 0,
      .key = cgpui::ElementKey{.value = "alpha"},
      .element_id = cgpui::ElementId{10},
      .content_bounds =
          cgpui::Rect{.origin = {.x = 0.0F, .y = 0.0F},
                       .size = {.width = 60.0F, .height = 22.0F}},
  };
  const cgpui::UniformListItemMeasurementResult second =
      cache.measure(resized_alpha);
  if (!second.cache_hit || cache.entry_count() != 1 ||
      cache.lookup_count() != 2 || cache.miss_count() != 1 ||
      cache.hit_count() != 1 || second.measurement.size.height != 22.0F) {
    return 19;
  }

  const std::optional<cgpui::UniformListItemMeasurement> cached =
      cache.measurement_for(cgpui::ElementKey{.value = "alpha"});
  if (!cached.has_value() || cached->size.height != 22.0F ||
      cached->element_id != cgpui::ElementId{10}) {
    return 20;
  }

  const std::vector<cgpui::UniformListItemIdentity> batch{
      resized_alpha,
      {
          .index = 1,
          .key = cgpui::ElementKey{.value = "beta"},
          .element_id = cgpui::ElementId{11},
          .content_bounds =
              cgpui::Rect{.origin = {.x = 0.0F, .y = 24.0F},
                           .size = {.width = 60.0F, .height = 12.0F}},
      },
  };
  const std::vector<cgpui::UniformListItemMeasurementResult> measured =
      cgpui::measure_uniform_list_items(cache, batch);
  return measured.size() == 2 && measured[0].cache_hit &&
                 !measured[1].cache_hit && cache.entry_count() == 2 &&
                 cache.lookup_count() == 4 && cache.miss_count() == 2 &&
                 cache.hit_count() == 2
             ? 0
             : 21;
}

int test_uniform_list_recycling_window_expands_visible_range_with_overscan() {
  cgpui::UniformListItemMeasurementCache cache;
  std::vector<cgpui::UniformListItemIdentity> items;
  for (std::size_t index = 0; index < 8; ++index) {
    items.push_back(cgpui::UniformListItemIdentity{
        .index = index,
        .key = cgpui::ElementKey{.value = std::to_string(index)},
        .element_id = cgpui::ElementId{100 + index},
        .content_bounds =
            cgpui::Rect{.origin = {.x = 0.0F,
                                   .y = static_cast<float>(index) * 10.0F},
                         .size = {.width = 40.0F, .height = 10.0F}},
    });
  }
  const std::vector<cgpui::UniformListItemMeasurementResult> measurements =
      cgpui::measure_uniform_list_items(cache, items);

  const cgpui::UniformListRecyclingWindow window =
      cgpui::calculate_uniform_list_recycling_window(
          cgpui::UniformListVisibleRange{.start_index = 3, .end_index = 5},
          measurements,
          1);
  if (window.retained_range.start_index != 2 ||
      window.retained_range.end_index != 6 || window.visible_count != 2 ||
      window.retained_count != 4 || window.recycled_before_count != 2 ||
      window.recycled_after_count != 2 || window.empty()) {
    return 22;
  }
  if (!window.retains(2) || !window.retains(5) || window.retains(1) ||
      !window.recycles(7) || window.recycles(4)) {
    return 23;
  }
  if (window.recycled_before_size.width != 40.0F ||
      window.recycled_before_size.height != 20.0F ||
      window.recycled_after_size.width != 40.0F ||
      window.recycled_after_size.height != 20.0F) {
    return 24;
  }

  const cgpui::UniformListRecyclingWindow no_overscan =
      cgpui::calculate_uniform_list_recycling_window(
          cgpui::UniformListVisibleRange{.start_index = 3, .end_index = 5},
          measurements,
          0);
  return no_overscan.retained_range.start_index == 3 &&
                 no_overscan.retained_range.end_index == 5 &&
                 no_overscan.retained_count == 2 &&
                 no_overscan.recycled_before_count == 3 &&
                 no_overscan.recycled_after_count == 3
             ? 0
             : 25;
}

int test_uniform_list_selection_state_tracks_pointer_and_keyboard() {
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
  };

  cgpui::UniformListSelectionState state;
  if (!state.empty() || state.selected_index().has_value() ||
      state.selected_key().has_value()) {
    return 26;
  }

  const std::optional<cgpui::UniformListSelection> pointer_selection =
      cgpui::select_uniform_list_item_at_point(
          items,
          cgpui::Point{.x = 5.0F, .y = 13.0F},
          cgpui::UniformListSelectionSource::pointer);
  if (!pointer_selection.has_value() || pointer_selection->index != 1 ||
      pointer_selection->key.value != "beta" ||
      pointer_selection->element_id != cgpui::ElementId{11} ||
      pointer_selection->source !=
          cgpui::UniformListSelectionSource::pointer) {
    return 27;
  }

  state.set(*pointer_selection);
  if (state.empty() || state.selected_index() != 1 ||
      state.selected_key()->value != "beta" ||
      !state.selected(cgpui::ElementKey{.value = "beta"}) ||
      state.selected(cgpui::ElementKey{.value = "gamma"})) {
    return 28;
  }

  const std::optional<cgpui::UniformListSelection> next =
      cgpui::move_uniform_list_selection(
          items,
          state,
          cgpui::UniformListSelectionDirection::next);
  if (!next.has_value() || next->index != 2 || next->key.value != "gamma" ||
      next->source != cgpui::UniformListSelectionSource::keyboard) {
    return 29;
  }

  const std::optional<cgpui::UniformListSelection> first =
      cgpui::move_uniform_list_selection(
          items,
          state,
          cgpui::UniformListSelectionDirection::first);
  const std::optional<cgpui::UniformListSelection> previous =
      cgpui::move_uniform_list_selection(
          items,
          state,
          cgpui::UniformListSelectionDirection::previous);
  return first.has_value() && first->index == 0 && previous.has_value() &&
                 previous->index == 0
             ? 0
             : 30;
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
  if (const int result = test_scroll_model_scrolls_rect_into_view();
      result != 0) {
    return result;
  }
  if (const int result = test_uniform_list_visible_range_uses_stable_item_bounds();
      result != 0) {
    return result;
  }
  if (const int result =
          test_uniform_list_scroll_anchor_preserves_keyed_item_position();
      result != 0) {
    return result;
  }
  if (const int result =
          test_uniform_list_item_measurement_cache_reuses_keyed_sizes();
      result != 0) {
    return result;
  }
  if (const int result =
          test_uniform_list_recycling_window_expands_visible_range_with_overscan();
      result != 0) {
    return result;
  }
  if (const int result =
          test_uniform_list_selection_state_tracks_pointer_and_keyboard();
      result != 0) {
    return result;
  }
  return 0;
}
