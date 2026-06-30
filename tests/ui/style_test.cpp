#include "cgpui/ui/style.hpp"

#include <concepts>

namespace {

bool same(float lhs, float rhs) {
  return lhs == rhs;
}

int test_edge_sizes_default_to_zero() {
  const cgpui::EdgeSizes edges;
  if (!same(edges.top, 0.0F) || !same(edges.right, 0.0F) ||
      !same(edges.bottom, 0.0F) || !same(edges.left, 0.0F)) {
    return 1;
  }
  return 0;
}

int test_edge_sizes_helpers_expand_values() {
  const cgpui::EdgeSizes all = cgpui::EdgeSizes::all(4.0F);
  if (!same(all.top, 4.0F) || !same(all.right, 4.0F) ||
      !same(all.bottom, 4.0F) || !same(all.left, 4.0F)) {
    return 2;
  }

  const cgpui::EdgeSizes axes =
      cgpui::EdgeSizes::axes(2.0F, 8.0F);
  if (!same(axes.top, 8.0F) || !same(axes.bottom, 8.0F) ||
      !same(axes.left, 2.0F) || !same(axes.right, 2.0F)) {
    return 3;
  }

  const cgpui::EdgeSizes explicit_edges =
      cgpui::EdgeSizes::trbl(1.0F, 2.0F, 3.0F, 4.0F);
  if (!same(explicit_edges.top, 1.0F) ||
      !same(explicit_edges.right, 2.0F) ||
      !same(explicit_edges.bottom, 3.0F) ||
      !same(explicit_edges.left, 4.0F)) {
    return 4;
  }

  return 0;
}

int test_style_unit_and_edge_authoring_helpers() {
  if (!same(cgpui::px(12.5F), 12.5F)) {
    return 31;
  }

  const cgpui::EdgeSizes all = cgpui::edges(3.0F);
  if (!same(all.top, 3.0F) || !same(all.right, 3.0F) ||
      !same(all.bottom, 3.0F) || !same(all.left, 3.0F)) {
    return 32;
  }

  const cgpui::EdgeSizes axes = cgpui::edges(6.0F, 2.0F);
  if (!same(axes.top, 2.0F) || !same(axes.right, 6.0F) ||
      !same(axes.bottom, 2.0F) || !same(axes.left, 6.0F)) {
    return 33;
  }

  const cgpui::EdgeSizes explicit_edges =
      cgpui::edges(1.0F, 2.0F, 3.0F, 4.0F);
  if (!same(explicit_edges.top, 1.0F) ||
      !same(explicit_edges.right, 2.0F) ||
      !same(explicit_edges.bottom, 3.0F) ||
      !same(explicit_edges.left, 4.0F)) {
    return 34;
  }

  return 0;
}

int test_color_authoring_helpers_normalize_channels() {
  const cgpui::Color red = cgpui::rgb(255, 0, 0);
  if (!same(red.r, 1.0F) || !same(red.g, 0.0F) ||
      !same(red.b, 0.0F) || !same(red.a, 1.0F)) {
    return 35;
  }

  const cgpui::Color orange = cgpui::rgba(255, 128, 0, 0.5F);
  if (!same(orange.r, 1.0F) ||
      !same(orange.g, 128.0F / 255.0F) ||
      !same(orange.b, 0.0F) || !same(orange.a, 0.5F)) {
    return 36;
  }

  return 0;
}

int test_border_radii_helpers_expand_values() {
  const cgpui::BorderRadii radii;
  if (!same(radii.top_left, 0.0F) || !same(radii.top_right, 0.0F) ||
      !same(radii.bottom_right, 0.0F) ||
      !same(radii.bottom_left, 0.0F)) {
    return 13;
  }

  const cgpui::BorderRadii all = cgpui::BorderRadii::all(6.0F);
  if (!same(all.top_left, 6.0F) || !same(all.top_right, 6.0F) ||
      !same(all.bottom_right, 6.0F) || !same(all.bottom_left, 6.0F)) {
    return 14;
  }

  const cgpui::BorderRadii corners =
      cgpui::BorderRadii::corners(1.0F, 2.0F, 3.0F, 4.0F);
  return same(corners.top_left, 1.0F) &&
                 same(corners.top_right, 2.0F) &&
                 same(corners.bottom_right, 3.0F) &&
                 same(corners.bottom_left, 4.0F)
             ? 0
             : 15;
}

int test_overflow_defaults_to_visible() {
  return cgpui::Style{}.overflow == cgpui::Overflow::visible ? 0 : 20;
}

int test_style_defaults_are_empty() {
  const cgpui::Style style;
  if (style.background_color.has_value() || style.foreground_color.has_value()) {
    return 5;
  }
  if (style.border_color.has_value()) {
    return 16;
  }
  if (style.clip_rect.has_value()) {
    return 21;
  }
  if (style.overflow != cgpui::Overflow::visible) {
    return 22;
  }
  if (!same(style.preferred_size.width, 0.0F) ||
      !same(style.preferred_size.height, 0.0F)) {
    return 6;
  }
  if (!same(style.padding.top, 0.0F) ||
      !same(style.border_width.left, 0.0F)) {
    return 7;
  }
  if (!same(style.margin.top, 0.0F) || !same(style.margin.left, 0.0F)) {
    return 29;
  }
  if (!same(style.border_radius.top_left, 0.0F) ||
      !same(style.border_radius.bottom_right, 0.0F)) {
    return 17;
  }
  if (style.z_index != 0) {
    return 25;
  }
  if (!same(style.gap, 0.0F)) {
    return 27;
  }
  if (style.align_items != cgpui::AlignItems::start) {
    return 48;
  }
  if (style.justify_content != cgpui::JustifyContent::start) {
    return 49;
  }
  return 0;
}

int test_style_builder_methods_store_values() {
  const cgpui::Style style =
      cgpui::Style{}
          .with_background_color(
              cgpui::Color{.r = 0.1F, .g = 0.2F, .b = 0.3F, .a = 0.4F})
          .with_foreground_color(
              cgpui::Color{.r = 0.5F, .g = 0.6F, .b = 0.7F, .a = 0.8F})
          .with_preferred_size(cgpui::Size{.width = 20.0F, .height = 30.0F})
          .with_padding(cgpui::EdgeSizes::axes(3.0F, 4.0F))
          .with_margin(cgpui::EdgeSizes::trbl(5.0F, 6.0F, 7.0F, 8.0F))
          .with_border_width(cgpui::EdgeSizes::all(2.0F))
          .with_border_color(
              cgpui::Color{.r = 0.9F, .g = 0.8F, .b = 0.7F, .a = 0.6F})
          .with_border_radius(cgpui::BorderRadii::corners(
              1.0F,
              2.0F,
              3.0F,
              4.0F))
          .with_overflow(cgpui::Overflow::hidden)
          .with_z_index(7)
          .with_gap(5.0F)
          .with_align_items(cgpui::AlignItems::center)
          .with_justify_content(cgpui::JustifyContent::space_between)
          .with_clip_rect(cgpui::Rect{
              .origin = {.x = 3.0F, .y = 4.0F},
              .size = {.width = 50.0F, .height = 60.0F},
          });

  if (!style.background_color.has_value() ||
      !same(style.background_color->r, 0.1F) ||
      !same(style.background_color->a, 0.4F)) {
    return 8;
  }
  if (!style.foreground_color.has_value() ||
      !same(style.foreground_color->g, 0.6F) ||
      !same(style.foreground_color->a, 0.8F)) {
    return 9;
  }
  if (!same(style.preferred_size.width, 20.0F) ||
      !same(style.preferred_size.height, 30.0F)) {
    return 10;
  }
  if (!same(style.padding.left, 3.0F) ||
      !same(style.padding.top, 4.0F)) {
    return 11;
  }
  if (!same(style.margin.top, 5.0F) ||
      !same(style.margin.right, 6.0F) ||
      !same(style.margin.bottom, 7.0F) ||
      !same(style.margin.left, 8.0F)) {
    return 30;
  }
  if (!same(style.border_width.right, 2.0F) ||
      !same(style.border_width.bottom, 2.0F)) {
    return 12;
  }
  if (!style.border_color.has_value() ||
      !same(style.border_color->r, 0.9F) ||
      !same(style.border_color->a, 0.6F)) {
    return 18;
  }
  if (!same(style.border_radius.top_left, 1.0F) ||
      !same(style.border_radius.top_right, 2.0F) ||
      !same(style.border_radius.bottom_right, 3.0F) ||
      !same(style.border_radius.bottom_left, 4.0F)) {
    return 19;
  }
  if (style.overflow != cgpui::Overflow::hidden) {
    return 23;
  }
  if (style.z_index != 7) {
    return 26;
  }
  if (!same(style.gap, 5.0F)) {
    return 28;
  }
  if (style.align_items != cgpui::AlignItems::center) {
    return 50;
  }
  if (style.justify_content != cgpui::JustifyContent::space_between) {
    return 51;
  }
  if (!style.clip_rect.has_value() ||
      !same(style.clip_rect->origin.x, 3.0F) ||
      !same(style.clip_rect->origin.y, 4.0F) ||
      !same(style.clip_rect->size.width, 50.0F) ||
      !same(style.clip_rect->size.height, 60.0F)) {
    return 24;
  }

  return 0;
}

int test_style_overlay_defaults_to_no_overrides() {
  const cgpui::StyleOverlay overlay;
  if (overlay.background_color.has_value() ||
      overlay.foreground_color.has_value() ||
      overlay.preferred_size.has_value() || overlay.padding.has_value() ||
      overlay.margin.has_value() || overlay.border_width.has_value() ||
      overlay.border_color.has_value() ||
      overlay.border_radius.has_value() || overlay.overflow.has_value() ||
      overlay.z_index.has_value() || overlay.gap.has_value() ||
      overlay.align_items.has_value() ||
      overlay.justify_content.has_value() ||
      overlay.clip_rect.has_value()) {
    return 37;
  }

  const cgpui::StyleOverlay authored =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(10, 20, 30))
          .with_foreground_color(cgpui::rgb(40, 50, 60))
          .with_preferred_size(cgpui::Size{.width = 11.0F, .height = 12.0F})
          .with_padding(cgpui::edges(1.0F))
          .with_margin(cgpui::edges(2.0F))
          .with_border_width(cgpui::edges(3.0F))
          .with_border_color(cgpui::rgb(70, 80, 90))
          .with_border_radius(cgpui::BorderRadii::all(4.0F))
          .with_overflow(cgpui::Overflow::hidden)
          .with_z_index(5)
          .with_gap(6.0F)
          .with_align_items(cgpui::AlignItems::end)
          .with_justify_content(cgpui::JustifyContent::center)
          .with_clip_rect(cgpui::Rect{
              .origin = {.x = 7.0F, .y = 8.0F},
              .size = {.width = 9.0F, .height = 10.0F},
          });

  if (!authored.background_color.has_value() ||
      authored.background_color->r != 10.0F / 255.0F ||
      !authored.foreground_color.has_value() ||
      authored.foreground_color->g != 50.0F / 255.0F) {
    return 38;
  }
  if (!authored.preferred_size.has_value() ||
      authored.preferred_size->width != 11.0F ||
      authored.preferred_size->height != 12.0F) {
    return 39;
  }
  if (!authored.padding.has_value() || authored.padding->left != 1.0F ||
      !authored.margin.has_value() || authored.margin->top != 2.0F ||
      !authored.border_width.has_value() ||
      authored.border_width->right != 3.0F) {
    return 40;
  }
  if (!authored.border_color.has_value() ||
      authored.border_color->b != 90.0F / 255.0F ||
      !authored.border_radius.has_value() ||
      authored.border_radius->bottom_left != 4.0F) {
    return 41;
  }
  if (!authored.overflow.has_value() ||
      *authored.overflow != cgpui::Overflow::hidden ||
      !authored.z_index.has_value() || *authored.z_index != 5 ||
      !authored.gap.has_value() || *authored.gap != 6.0F) {
    return 42;
  }
  if (!authored.align_items.has_value() ||
      *authored.align_items != cgpui::AlignItems::end ||
      !authored.justify_content.has_value() ||
      *authored.justify_content != cgpui::JustifyContent::center) {
    return 52;
  }
  return authored.clip_rect.has_value() &&
                 authored.clip_rect->origin.x == 7.0F &&
                 authored.clip_rect->size.height == 10.0F
             ? 0
             : 43;
}

int test_style_state_resolves_hover_focus_disabled_order() {
  cgpui::StyleState state;
  state.base = cgpui::Style{}
                   .with_background_color(cgpui::rgb(10, 10, 10))
                   .with_foreground_color(cgpui::rgb(20, 20, 20))
                   .with_preferred_size(
                       cgpui::Size{.width = 100.0F, .height = 30.0F})
                   .with_padding(cgpui::edges(2.0F))
                   .with_gap(1.0F)
                   .with_align_items(cgpui::AlignItems::start)
                   .with_justify_content(cgpui::JustifyContent::start);
  state.hover =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(30, 30, 30))
          .with_padding(cgpui::edges(4.0F))
          .with_gap(2.0F)
          .with_align_items(cgpui::AlignItems::center);
  state.focus =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(40, 40, 40))
          .with_foreground_color(cgpui::rgb(50, 50, 50))
          .with_justify_content(cgpui::JustifyContent::end);
  state.disabled =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(60, 60, 60))
          .with_border_width(cgpui::edges(3.0F))
          .with_align_items(cgpui::AlignItems::end);

  const cgpui::Style hover =
      cgpui::resolved_style(state, cgpui::StyleStateFlags{.hovered = true});
  if (!hover.background_color.has_value() ||
      hover.background_color->r != 30.0F / 255.0F ||
      hover.foreground_color->r != 20.0F / 255.0F ||
      hover.padding.top != 4.0F || hover.gap != 2.0F ||
      hover.align_items != cgpui::AlignItems::center ||
      hover.justify_content != cgpui::JustifyContent::start) {
    return 44;
  }

  const cgpui::Style focused = cgpui::resolved_style(
      state,
      cgpui::StyleStateFlags{.hovered = true, .focused = true});
  if (!focused.background_color.has_value() ||
      focused.background_color->r != 40.0F / 255.0F ||
      !focused.foreground_color.has_value() ||
      focused.foreground_color->r != 50.0F / 255.0F ||
      focused.padding.top != 4.0F || focused.gap != 2.0F ||
      focused.align_items != cgpui::AlignItems::center ||
      focused.justify_content != cgpui::JustifyContent::end) {
    return 45;
  }

  const cgpui::Style disabled = cgpui::resolved_style(
      state,
      cgpui::StyleStateFlags{
          .hovered = true,
          .focused = true,
          .disabled = true,
      });
  if (!disabled.background_color.has_value() ||
      disabled.background_color->r != 60.0F / 255.0F ||
      !disabled.foreground_color.has_value() ||
      disabled.foreground_color->r != 50.0F / 255.0F ||
      disabled.padding.top != 4.0F || disabled.gap != 2.0F ||
      disabled.border_width.left != 3.0F ||
      disabled.align_items != cgpui::AlignItems::end ||
      disabled.justify_content != cgpui::JustifyContent::end) {
    return 46;
  }

  return disabled.preferred_size.width == 100.0F &&
                 disabled.preferred_size.height == 30.0F
             ? 0
             : 47;
}

} // namespace

static_assert(std::same_as<decltype(cgpui::EdgeSizes{}.top), float>);
static_assert(std::same_as<decltype(cgpui::BorderRadii{}.top_left), float>);
static_assert(std::same_as<decltype(cgpui::Style{}.overflow), cgpui::Overflow>);
static_assert(std::same_as<decltype(cgpui::Style{}.z_index), int>);
static_assert(std::same_as<decltype(cgpui::Style{}.gap), float>);
static_assert(
    std::same_as<decltype(cgpui::Style{}.align_items), cgpui::AlignItems>);
static_assert(std::same_as<
              decltype(cgpui::Style{}.justify_content),
              cgpui::JustifyContent>);
static_assert(std::same_as<decltype(cgpui::Style{}.margin), cgpui::EdgeSizes>);
static_assert(std::same_as<decltype(cgpui::px(1.0F)), float>);
static_assert(std::same_as<decltype(cgpui::rgb(255, 255, 255)), cgpui::Color>);
static_assert(
    std::same_as<decltype(cgpui::rgba(255, 255, 255, 1.0F)), cgpui::Color>);
static_assert(std::same_as<decltype(cgpui::edges(1.0F)), cgpui::EdgeSizes>);
static_assert(std::same_as<
              decltype(cgpui::StyleOverlay{}.preferred_size),
              std::optional<cgpui::Size>>);
static_assert(std::same_as<decltype(cgpui::StyleState{}.base), cgpui::Style>);
static_assert(std::same_as<
              decltype(cgpui::StyleStateFlags{}.hovered),
              bool>);

int main() {
  if (const int result = test_edge_sizes_default_to_zero(); result != 0) {
    return result;
  }
  if (const int result = test_edge_sizes_helpers_expand_values();
      result != 0) {
    return result;
  }
  if (const int result = test_style_unit_and_edge_authoring_helpers();
      result != 0) {
    return result;
  }
  if (const int result = test_color_authoring_helpers_normalize_channels();
      result != 0) {
    return result;
  }
  if (const int result = test_border_radii_helpers_expand_values();
      result != 0) {
    return result;
  }
  if (const int result = test_overflow_defaults_to_visible(); result != 0) {
    return result;
  }
  if (const int result = test_style_defaults_are_empty(); result != 0) {
    return result;
  }
  if (const int result = test_style_builder_methods_store_values();
      result != 0) {
    return result;
  }
  if (const int result = test_style_overlay_defaults_to_no_overrides();
      result != 0) {
    return result;
  }
  if (const int result = test_style_state_resolves_hover_focus_disabled_order();
      result != 0) {
    return result;
  }
  return 0;
}
