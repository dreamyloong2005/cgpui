#include "cgpui/ui/style.hpp"

#include <cmath>
#include <concepts>
#include <span>
#include <string>

namespace {

bool same(float lhs, float rhs) {
  return lhs == rhs;
}

bool same_transform(cgpui::AffineTransform lhs, cgpui::AffineTransform rhs) {
  return lhs.scale_x == rhs.scale_x && lhs.skew_y == rhs.skew_y &&
         lhs.skew_x == rhs.skew_x && lhs.scale_y == rhs.scale_y &&
         lhs.translate_x == rhs.translate_x &&
         lhs.translate_y == rhs.translate_y;
}

bool same_shadow(const cgpui::BoxShadow& lhs, const cgpui::BoxShadow& rhs) {
  return lhs.color.r == rhs.color.r && lhs.color.g == rhs.color.g &&
         lhs.color.b == rhs.color.b && lhs.color.a == rhs.color.a &&
         lhs.offset.x == rhs.offset.x && lhs.offset.y == rhs.offset.y &&
         lhs.blur_radius == rhs.blur_radius &&
         lhs.spread_radius == rhs.spread_radius;
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
  if (style.font.family != "" || !same(style.font_size, 16.0F)) {
    return 61;
  }
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
  if (!same(style.min_size.width, 0.0F) ||
      !same(style.min_size.height, 0.0F) ||
      !std::isinf(style.max_size.width) ||
      !std::isinf(style.max_size.height)) {
    return 87;
  }
  if (style.percentage_size.width.has_value() ||
      style.percentage_size.height.has_value()) {
    return 91;
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
  if (style.layer != 0) {
    return 59;
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
  if (!same(style.flex_grow, 0.0F) || !same(style.flex_shrink, 0.0F)) {
    return 53;
  }
  if (style.position != cgpui::Position::relative ||
      !same(style.inset.top, 0.0F) || !same(style.inset.left, 0.0F)) {
    return 56;
  }
  if (!same(style.opacity, 1.0F) ||
      !same_transform(style.transform, cgpui::AffineTransform::identity())) {
    return 78;
  }
  if (style.box_shadow.has_value()) {
    return 81;
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
          .with_min_size(cgpui::Size{.width = 10.0F, .height = 15.0F})
          .with_max_size(cgpui::Size{.width = 120.0F, .height = 130.0F})
          .with_percentage_size(
              cgpui::PercentageSize{.width = 25.0F, .height = 50.0F})
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
          .with_layer(4)
          .with_gap(5.0F)
          .with_align_items(cgpui::AlignItems::center)
          .with_justify_content(cgpui::JustifyContent::space_between)
          .with_flex_grow(2.0F)
          .with_flex_shrink(3.0F)
          .with_position(cgpui::Position::absolute)
          .with_inset(cgpui::edges(9.0F, 10.0F, 11.0F, 12.0F))
          .with_font(cgpui::FontDescriptor{.family = "Inter"})
          .with_font_size(20.0F)
          .with_box_shadow(cgpui::BoxShadow{
              .color = cgpui::rgba(10, 20, 30, 0.35F),
              .offset = {.x = 2.0F, .y = 3.0F},
              .blur_radius = 12.0F,
              .spread_radius = 1.5F,
          })
          .with_opacity(0.5F)
          .with_transform(cgpui::AffineTransform::translation(6.0F, 7.0F))
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
  if (!same(style.min_size.width, 10.0F) ||
      !same(style.min_size.height, 15.0F) ||
      !same(style.max_size.width, 120.0F) ||
      !same(style.max_size.height, 130.0F)) {
    return 88;
  }
  if (!style.percentage_size.width.has_value() ||
      !style.percentage_size.height.has_value() ||
      !same(*style.percentage_size.width, 25.0F) ||
      !same(*style.percentage_size.height, 50.0F)) {
    return 92;
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
  if (style.layer != 4) {
    return 60;
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
  if (!same(style.flex_grow, 2.0F) || !same(style.flex_shrink, 3.0F)) {
    return 54;
  }
  if (style.position != cgpui::Position::absolute ||
      !same(style.inset.top, 9.0F) || !same(style.inset.right, 10.0F) ||
      !same(style.inset.bottom, 11.0F) || !same(style.inset.left, 12.0F)) {
    return 57;
  }
  if (style.font.family != "Inter" || !same(style.font_size, 20.0F)) {
    return 62;
  }
  if (!style.box_shadow.has_value() ||
      !same_shadow(
          *style.box_shadow,
          cgpui::BoxShadow{
              .color = cgpui::rgba(10, 20, 30, 0.35F),
              .offset = {.x = 2.0F, .y = 3.0F},
              .blur_radius = 12.0F,
              .spread_radius = 1.5F,
          })) {
    return 82;
  }
  if (!same(style.opacity, 0.5F) ||
      !same_transform(
          style.transform,
          cgpui::AffineTransform::translation(6.0F, 7.0F))) {
    return 79;
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
      overlay.min_size.has_value() || overlay.max_size.has_value() ||
      overlay.percentage_size.width.has_value() ||
      overlay.percentage_size.height.has_value() ||
      overlay.margin.has_value() || overlay.border_width.has_value() ||
      overlay.border_color.has_value() ||
      overlay.border_radius.has_value() || overlay.overflow.has_value() ||
      overlay.z_index.has_value() || overlay.layer.has_value() ||
      overlay.gap.has_value() ||
      overlay.align_items.has_value() ||
      overlay.justify_content.has_value() ||
      overlay.flex_grow.has_value() || overlay.flex_shrink.has_value() ||
      overlay.position.has_value() || overlay.inset.has_value() ||
      overlay.font.has_value() || overlay.font_size.has_value() ||
      overlay.box_shadow.has_value() ||
      overlay.opacity.has_value() || overlay.transform.has_value() ||
      overlay.clip_rect.has_value()) {
    return 37;
  }

  const cgpui::StyleOverlay authored =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(10, 20, 30))
          .with_foreground_color(cgpui::rgb(40, 50, 60))
          .with_preferred_size(cgpui::Size{.width = 11.0F, .height = 12.0F})
          .with_min_size(cgpui::Size{.width = 21.0F, .height = 22.0F})
          .with_max_size(cgpui::Size{.width = 111.0F, .height = 112.0F})
          .with_width_percent(33.0F)
          .with_height_percent(66.0F)
          .with_padding(cgpui::edges(1.0F))
          .with_margin(cgpui::edges(2.0F))
          .with_border_width(cgpui::edges(3.0F))
          .with_border_color(cgpui::rgb(70, 80, 90))
          .with_border_radius(cgpui::BorderRadii::all(4.0F))
          .with_overflow(cgpui::Overflow::hidden)
          .with_z_index(5)
          .with_layer(8)
          .with_gap(6.0F)
          .with_align_items(cgpui::AlignItems::end)
          .with_justify_content(cgpui::JustifyContent::center)
          .with_flex_grow(4.0F)
          .with_flex_shrink(5.0F)
          .with_position(cgpui::Position::absolute)
          .with_inset(cgpui::edges(13.0F))
          .with_font(cgpui::FontDescriptor{.family = "Serif"})
          .with_font_size(18.0F)
          .with_box_shadow(cgpui::BoxShadow{
              .color = cgpui::rgba(1, 2, 3, 0.5F),
              .offset = {.x = 4.0F, .y = 5.0F},
              .blur_radius = 6.0F,
              .spread_radius = 0.5F,
          })
          .with_opacity(0.25F)
          .with_transform(cgpui::AffineTransform::translation(9.0F, 10.0F))
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
  if (!authored.min_size.has_value() ||
      authored.min_size->width != 21.0F ||
      authored.min_size->height != 22.0F ||
      !authored.max_size.has_value() ||
      authored.max_size->width != 111.0F ||
      authored.max_size->height != 112.0F) {
    return 89;
  }
  if (!authored.percentage_size.width.has_value() ||
      !authored.percentage_size.height.has_value() ||
      authored.percentage_size.width != 33.0F ||
      authored.percentage_size.height != 66.0F) {
    return 93;
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
      !authored.layer.has_value() || *authored.layer != 8 ||
      !authored.gap.has_value() || *authored.gap != 6.0F) {
    return 42;
  }
  if (!authored.align_items.has_value() ||
      *authored.align_items != cgpui::AlignItems::end ||
      !authored.justify_content.has_value() ||
      *authored.justify_content != cgpui::JustifyContent::center) {
    return 52;
  }
  if (!authored.flex_grow.has_value() || *authored.flex_grow != 4.0F ||
      !authored.flex_shrink.has_value() || *authored.flex_shrink != 5.0F) {
    return 55;
  }
  if (!authored.position.has_value() ||
      *authored.position != cgpui::Position::absolute ||
      !authored.inset.has_value() || authored.inset->left != 13.0F) {
    return 58;
  }
  if (!authored.font.has_value() || authored.font->family != "Serif" ||
      !authored.font_size.has_value() || *authored.font_size != 18.0F) {
    return 63;
  }
  if (!authored.box_shadow.has_value() ||
      !same_shadow(
          *authored.box_shadow,
          cgpui::BoxShadow{
              .color = cgpui::rgba(1, 2, 3, 0.5F),
              .offset = {.x = 4.0F, .y = 5.0F},
              .blur_radius = 6.0F,
              .spread_radius = 0.5F,
          })) {
    return 83;
  }
  if (!authored.opacity.has_value() || *authored.opacity != 0.25F ||
      !authored.transform.has_value() ||
      !same_transform(
          *authored.transform,
          cgpui::AffineTransform::translation(9.0F, 10.0F))) {
    return 80;
  }
  return authored.clip_rect.has_value() &&
                 authored.clip_rect->origin.x == 7.0F &&
                 authored.clip_rect->size.height == 10.0F
             ? 0
             : 43;
}

int test_style_state_resolves_hover_focus_active_disabled_order() {
  cgpui::StyleState state;
  state.base = cgpui::Style{}
                   .with_background_color(cgpui::rgb(10, 10, 10))
                   .with_foreground_color(cgpui::rgb(20, 20, 20))
                   .with_preferred_size(
                       cgpui::Size{.width = 100.0F, .height = 30.0F})
                   .with_min_size(
                       cgpui::Size{.width = 10.0F, .height = 20.0F})
                   .with_max_size(
                       cgpui::Size{.width = 200.0F, .height = 220.0F})
                   .with_percentage_size(
                       cgpui::PercentageSize{.width = 10.0F, .height = 20.0F})
                   .with_padding(cgpui::edges(2.0F))
                   .with_gap(1.0F)
                   .with_align_items(cgpui::AlignItems::start)
                   .with_justify_content(cgpui::JustifyContent::start)
                   .with_flex_grow(1.0F)
                   .with_flex_shrink(1.0F)
                   .with_layer(1)
                   .with_position(cgpui::Position::relative)
                   .with_inset(cgpui::edges(1.0F))
                   .with_font(cgpui::FontDescriptor{.family = "Base"})
                   .with_font_size(14.0F)
                   .with_box_shadow(cgpui::BoxShadow{
                       .color = cgpui::rgba(0, 0, 0, 0.2F),
                       .offset = {.x = 1.0F, .y = 2.0F},
                       .blur_radius = 3.0F,
                       .spread_radius = 0.0F,
                   })
                   .with_opacity(0.9F)
                   .with_transform(
                       cgpui::AffineTransform::translation(2.0F, 3.0F));
  state.hover =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(30, 30, 30))
          .with_min_size(cgpui::Size{.width = 30.0F, .height = 40.0F})
          .with_width_percent(30.0F)
          .with_padding(cgpui::edges(4.0F))
          .with_gap(2.0F)
          .with_align_items(cgpui::AlignItems::center)
          .with_flex_grow(2.0F)
          .with_layer(2)
          .with_position(cgpui::Position::absolute)
          .with_font_size(18.0F)
          .with_box_shadow(cgpui::BoxShadow{
              .color = cgpui::rgba(10, 10, 10, 0.3F),
              .offset = {.x = 2.0F, .y = 3.0F},
              .blur_radius = 4.0F,
              .spread_radius = 1.0F,
          })
          .with_opacity(0.7F);
  state.focus =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(40, 40, 40))
          .with_foreground_color(cgpui::rgb(50, 50, 50))
          .with_max_size(cgpui::Size{.width = 150.0F, .height = 160.0F})
          .with_height_percent(40.0F)
          .with_justify_content(cgpui::JustifyContent::end)
          .with_flex_shrink(3.0F)
          .with_layer(3)
          .with_inset(cgpui::edges(4.0F))
          .with_font(cgpui::FontDescriptor{.family = "Focus"})
          .with_transform(
              cgpui::AffineTransform::translation(5.0F, 6.0F));
  state.active =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(70, 70, 70))
          .with_foreground_color(cgpui::rgb(80, 80, 80))
          .with_gap(8.0F)
          .with_flex_grow(5.0F)
          .with_layer(5)
          .with_opacity(0.5F)
          .with_transform(
              cgpui::AffineTransform::translation(7.0F, 8.0F));
  state.disabled =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(60, 60, 60))
          .with_min_size(cgpui::Size{.width = 50.0F, .height = 60.0F})
          .with_border_width(cgpui::edges(3.0F))
          .with_layer(4)
          .with_align_items(cgpui::AlignItems::end)
          .with_font_size(12.0F)
          .with_opacity(0.25F);

  const cgpui::Style hover =
      cgpui::resolved_style(state, cgpui::StyleStateFlags{.hovered = true});
  if (!hover.background_color.has_value() ||
      hover.background_color->r != 30.0F / 255.0F ||
      hover.foreground_color->r != 20.0F / 255.0F ||
      hover.padding.top != 4.0F || hover.gap != 2.0F ||
      hover.align_items != cgpui::AlignItems::center ||
      hover.justify_content != cgpui::JustifyContent::start ||
      hover.flex_grow != 2.0F || hover.flex_shrink != 1.0F ||
      hover.layer != 2 ||
      hover.position != cgpui::Position::absolute ||
      hover.inset.left != 1.0F ||
      hover.min_size.width != 30.0F || hover.min_size.height != 40.0F ||
      hover.max_size.width != 200.0F || hover.max_size.height != 220.0F ||
      !hover.percentage_size.width.has_value() ||
      !hover.percentage_size.height.has_value() ||
      *hover.percentage_size.width != 30.0F ||
      *hover.percentage_size.height != 20.0F ||
      hover.font.family != "Base" || hover.font_size != 18.0F ||
      !hover.box_shadow.has_value() ||
      !same_shadow(
          *hover.box_shadow,
          cgpui::BoxShadow{
              .color = cgpui::rgba(10, 10, 10, 0.3F),
              .offset = {.x = 2.0F, .y = 3.0F},
              .blur_radius = 4.0F,
              .spread_radius = 1.0F,
          }) ||
      hover.opacity != 0.7F ||
      !same_transform(
          hover.transform,
          cgpui::AffineTransform::translation(2.0F, 3.0F))) {
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
      focused.justify_content != cgpui::JustifyContent::end ||
      focused.flex_grow != 2.0F || focused.flex_shrink != 3.0F ||
      focused.layer != 3 ||
      focused.position != cgpui::Position::absolute ||
      focused.inset.left != 4.0F ||
      focused.min_size.width != 30.0F ||
      focused.min_size.height != 40.0F ||
      focused.max_size.width != 150.0F ||
      focused.max_size.height != 160.0F ||
      !focused.percentage_size.width.has_value() ||
      !focused.percentage_size.height.has_value() ||
      *focused.percentage_size.width != 30.0F ||
      *focused.percentage_size.height != 40.0F ||
      focused.font.family != "Focus" || focused.font_size != 18.0F ||
      !focused.box_shadow.has_value() ||
      !same_shadow(
          *focused.box_shadow,
          cgpui::BoxShadow{
              .color = cgpui::rgba(10, 10, 10, 0.3F),
              .offset = {.x = 2.0F, .y = 3.0F},
              .blur_radius = 4.0F,
              .spread_radius = 1.0F,
          }) ||
      focused.opacity != 0.7F ||
      !same_transform(
          focused.transform,
          cgpui::AffineTransform::translation(5.0F, 6.0F))) {
    return 45;
  }

  const cgpui::Style active = cgpui::resolved_style(
      state,
      cgpui::StyleStateFlags{
          .hovered = true,
          .focused = true,
          .active = true,
      });
  if (!active.background_color.has_value() ||
      active.background_color->r != 70.0F / 255.0F ||
      !active.foreground_color.has_value() ||
      active.foreground_color->r != 80.0F / 255.0F ||
      active.padding.top != 4.0F || active.gap != 8.0F ||
      active.align_items != cgpui::AlignItems::center ||
      active.justify_content != cgpui::JustifyContent::end ||
      active.flex_grow != 5.0F || active.flex_shrink != 3.0F ||
      active.layer != 5 ||
      active.position != cgpui::Position::absolute ||
      active.inset.left != 4.0F ||
      active.min_size.width != 30.0F ||
      active.min_size.height != 40.0F ||
      active.max_size.width != 150.0F ||
      active.max_size.height != 160.0F ||
      !active.percentage_size.width.has_value() ||
      !active.percentage_size.height.has_value() ||
      *active.percentage_size.width != 30.0F ||
      *active.percentage_size.height != 40.0F ||
      active.font.family != "Focus" || active.font_size != 18.0F ||
      !active.box_shadow.has_value() ||
      !same_shadow(
          *active.box_shadow,
          cgpui::BoxShadow{
              .color = cgpui::rgba(10, 10, 10, 0.3F),
              .offset = {.x = 2.0F, .y = 3.0F},
              .blur_radius = 4.0F,
              .spread_radius = 1.0F,
          }) ||
      active.opacity != 0.5F ||
      !same_transform(
          active.transform,
          cgpui::AffineTransform::translation(7.0F, 8.0F))) {
    return 48;
  }

  const cgpui::Style disabled = cgpui::resolved_style(
      state,
      cgpui::StyleStateFlags{
          .hovered = true,
          .focused = true,
          .active = true,
          .disabled = true,
      });
  if (!disabled.background_color.has_value() ||
      disabled.background_color->r != 60.0F / 255.0F ||
      !disabled.foreground_color.has_value() ||
      disabled.foreground_color->r != 80.0F / 255.0F ||
      disabled.padding.top != 4.0F || disabled.gap != 8.0F ||
      disabled.border_width.left != 3.0F ||
      disabled.align_items != cgpui::AlignItems::end ||
      disabled.justify_content != cgpui::JustifyContent::end ||
      disabled.flex_grow != 5.0F || disabled.flex_shrink != 3.0F ||
      disabled.layer != 4 ||
      disabled.position != cgpui::Position::absolute ||
      disabled.inset.left != 4.0F ||
      disabled.min_size.width != 50.0F ||
      disabled.min_size.height != 60.0F ||
      disabled.max_size.width != 150.0F ||
      disabled.max_size.height != 160.0F ||
      disabled.font.family != "Focus" || disabled.font_size != 12.0F ||
      !disabled.box_shadow.has_value() ||
      !same_shadow(
          *disabled.box_shadow,
          cgpui::BoxShadow{
              .color = cgpui::rgba(10, 10, 10, 0.3F),
              .offset = {.x = 2.0F, .y = 3.0F},
              .blur_radius = 4.0F,
              .spread_radius = 1.0F,
          }) ||
      disabled.opacity != 0.25F ||
      !same_transform(
          disabled.transform,
          cgpui::AffineTransform::translation(7.0F, 8.0F))) {
    return 46;
  }

  return disabled.preferred_size.width == 100.0F &&
                 disabled.preferred_size.height == 30.0F
             ? 0
             : 47;
}

int test_style_classes_store_named_class_ids() {
  const cgpui::StyleClassId primary = cgpui::style_class("button.primary");
  const cgpui::StyleClassId secondary = cgpui::style_class("button.secondary");
  const cgpui::StyleClassId primary_again =
      cgpui::style_class("button.primary");

  if (primary.value != "button.primary" || primary != primary_again ||
      primary == secondary) {
    return 64;
  }

  cgpui::StyleClasses classes;
  if (!classes.empty() || classes.contains(primary)) {
    return 65;
  }

  classes.add(primary).add(secondary).add(primary_again);
  if (classes.empty() || classes.size() != 2 ||
      !classes.contains(primary) || !classes.contains(secondary)) {
    return 66;
  }

  const std::span<const cgpui::StyleClassId> ids = classes.ids();
  return ids.size() == 2 && ids[0] == primary && ids[1] == secondary ? 0 : 67;
}

int test_theme_color_and_spacing_tokens_lookup_softly() {
  const cgpui::ThemeTokenId accent = cgpui::theme_token("color.accent");
  const cgpui::ThemeTokenId gap = cgpui::theme_token("space.gap");
  const cgpui::ThemeTokenId missing = cgpui::theme_token("missing");

  if (accent.value != "color.accent" || accent == gap) {
    return 68;
  }

  cgpui::Theme theme;
  if (theme.color(accent).has_value() || theme.spacing(gap).has_value()) {
    return 69;
  }

  theme.set_color(accent, cgpui::rgb(10, 20, 30))
      .set_spacing(gap, cgpui::px(8.0F));
  const std::optional<cgpui::Color> accent_color = theme.color(accent);
  const std::optional<float> gap_spacing = theme.spacing(gap);
  if (!accent_color.has_value() || accent_color->r != 10.0F / 255.0F ||
      !gap_spacing.has_value() || *gap_spacing != 8.0F) {
    return 70;
  }

  theme.set_color(accent, cgpui::rgb(40, 50, 60))
      .set_spacing(gap, cgpui::px(12.0F));
  const std::optional<cgpui::Color> replaced_color = theme.color(accent);
  const std::optional<float> replaced_spacing = theme.spacing(gap);
  if (!replaced_color.has_value() ||
      replaced_color->r != 40.0F / 255.0F ||
      !replaced_spacing.has_value() || *replaced_spacing != 12.0F) {
    return 71;
  }

  return !theme.color(missing).has_value() &&
                 !theme.spacing(missing).has_value()
             ? 0
             : 72;
}

int test_style_cascade_resolves_base_classes_state_and_inline_order() {
  const cgpui::StyleClassId base_class = cgpui::style_class("surface.card");
  const cgpui::StyleClassId accent_class = cgpui::style_class("accented");
  const cgpui::StyleClassId missing_class = cgpui::style_class("missing");

  cgpui::StyleCascade cascade;
  cascade.set_class_style(
      base_class,
      cgpui::StyleState{
          .base = cgpui::Style{}
                      .with_background_color(cgpui::rgb(20, 20, 20))
                      .with_min_size(
                          cgpui::Size{.width = 30.0F, .height = 40.0F})
                      .with_padding(cgpui::edges(6.0F))
                      .with_gap(2.0F)
                      .with_box_shadow(cgpui::BoxShadow{
                          .color = cgpui::rgba(20, 20, 20, 0.25F),
                          .offset = {.x = 1.0F, .y = 2.0F},
                          .blur_radius = 8.0F,
                          .spread_radius = 0.5F,
                      })
                      .with_opacity(0.8F),
          .hover = cgpui::StyleOverlay{}
                       .with_background_color(cgpui::rgb(30, 30, 30))
                       .with_gap(4.0F)
                       .with_transform(
                           cgpui::AffineTransform::translation(2.0F, 3.0F)),
          .active = cgpui::StyleOverlay{}
                        .with_foreground_color(cgpui::rgb(55, 55, 55))
                        .with_gap(5.0F),
      });
  cascade.set_class_style(
      accent_class,
      cgpui::StyleState{
          .base = cgpui::Style{}
                      .with_foreground_color(cgpui::rgb(80, 90, 100))
                      .with_max_size(
                          cgpui::Size{.width = 90.0F, .height = 100.0F})
                      .with_padding(cgpui::edges(8.0F))
                      .with_transform(
                          cgpui::AffineTransform::translation(4.0F, 5.0F)),
          .focus = cgpui::StyleOverlay{}
                       .with_background_color(cgpui::rgb(40, 40, 40))
                       .with_border_width(cgpui::edges(3.0F)),
          .active = cgpui::StyleOverlay{}
                        .with_background_color(cgpui::rgb(50, 50, 50))
                        .with_opacity(0.7F),
      });

  cgpui::StyleClasses classes;
  classes.add(base_class).add(missing_class).add(accent_class);
  const cgpui::StyleState local{
      .base = cgpui::Style{}
                  .with_background_color(cgpui::rgb(10, 10, 10))
                  .with_preferred_size(
                      cgpui::Size{.width = 10.0F, .height = 20.0F})
                  .with_min_size(
                      cgpui::Size{.width = 5.0F, .height = 6.0F})
                  .with_max_size(
                      cgpui::Size{.width = 300.0F, .height = 400.0F})
                  .with_percentage_size(
                      cgpui::PercentageSize{.width = 25.0F, .height = 50.0F})
                  .with_padding(cgpui::edges(1.0F)),
      .hover = cgpui::StyleOverlay{}.with_gap(6.0F),
      .focus = cgpui::StyleOverlay{}.with_foreground_color(
          cgpui::rgb(120, 130, 140)),
      .active = cgpui::StyleOverlay{}
                    .with_foreground_color(cgpui::rgb(150, 160, 170))
                    .with_gap(7.0F),
  };
  const cgpui::StyleOverlay inline_style =
      cgpui::StyleOverlay{}
          .with_background_color(cgpui::rgb(200, 10, 10))
          .with_max_size(cgpui::Size{.width = 70.0F, .height = 80.0F})
          .with_width_percent(75.0F)
          .with_padding(cgpui::edges(12.0F))
          .with_opacity(0.6F);

  const cgpui::Style resolved = cgpui::resolved_style(
      cascade,
      local,
      classes,
      inline_style,
      cgpui::StyleStateFlags{
          .hovered = true,
          .focused = true,
          .active = true,
      });

  if (!resolved.background_color.has_value() ||
      resolved.background_color->r != 200.0F / 255.0F) {
    return 73;
  }
  if (!resolved.foreground_color.has_value() ||
      resolved.foreground_color->r != 150.0F / 255.0F) {
    return 74;
  }
  if (resolved.padding.left != 12.0F || resolved.gap != 7.0F) {
    return 75;
  }
  if (resolved.border_width.left != 3.0F ||
      resolved.preferred_size.width != 10.0F ||
      resolved.preferred_size.height != 20.0F) {
    return 76;
  }
  if (resolved.min_size.width != 30.0F ||
      resolved.min_size.height != 40.0F ||
      resolved.max_size.width != 70.0F ||
      resolved.max_size.height != 80.0F) {
    return 90;
  }
  if (!resolved.percentage_size.width.has_value() ||
      !resolved.percentage_size.height.has_value() ||
      *resolved.percentage_size.width != 75.0F ||
      *resolved.percentage_size.height != 50.0F) {
    return 94;
  }
  if (resolved.opacity != 0.6F ||
      !same_transform(
          resolved.transform,
          cgpui::AffineTransform::translation(4.0F, 5.0F))) {
    return 81;
  }
  if (!resolved.box_shadow.has_value() ||
      !same_shadow(
          *resolved.box_shadow,
          cgpui::BoxShadow{
              .color = cgpui::rgba(20, 20, 20, 0.25F),
              .offset = {.x = 1.0F, .y = 2.0F},
              .blur_radius = 8.0F,
              .spread_radius = 0.5F,
          })) {
    return 86;
  }

  const cgpui::StyleState* accent_rule = cascade.class_style(accent_class);
  return accent_rule != nullptr &&
                 accent_rule->base.foreground_color.has_value()
             ? 0
             : 77;
}

int test_style_cascade_reuses_class_rules_depth_first_and_ignores_cycles() {
  const cgpui::StyleClassId surface = cgpui::style_class("surface");
  const cgpui::StyleClassId compact = cgpui::style_class("compact");
  const cgpui::StyleClassId card = cgpui::style_class("card");
  const cgpui::StyleClassId cycle_a = cgpui::style_class("cycle.a");
  const cgpui::StyleClassId cycle_b = cgpui::style_class("cycle.b");

  cgpui::StyleCascade cascade;
  cascade.set_class_style(
      surface,
      cgpui::StyleState{
          .base = cgpui::Style{}
                      .with_background_color(cgpui::rgb(12, 24, 36))
                      .with_padding(cgpui::edges(2.0F)),
          .hover = cgpui::StyleOverlay{}.with_gap(3.0F),
          .active = cgpui::StyleOverlay{}.with_foreground_color(
              cgpui::rgb(50, 60, 70)),
      });

  cgpui::StyleClasses compact_reuse;
  compact_reuse.add(surface);
  cascade.set_class_rule(
      compact,
      cgpui::StyleClassRule{
          .reused_classes = compact_reuse,
          .style = cgpui::StyleState{
              .base = cgpui::Style{}.with_padding(cgpui::edges(6.0F)),
              .focus =
                  cgpui::StyleOverlay{}.with_border_width(cgpui::edges(4.0F)),
          },
      });

  cgpui::StyleClasses card_reuse;
  card_reuse.add(compact);
  cascade.set_class_rule(
      card,
      cgpui::StyleClassRule{
          .reused_classes = card_reuse,
          .style = cgpui::StyleState{
              .base = cgpui::Style{}.with_opacity(0.75F),
              .active = cgpui::StyleOverlay{}.with_gap(8.0F),
          },
      });

  cgpui::StyleClasses cycle_a_reuse;
  cycle_a_reuse.add(cycle_b);
  cascade.set_class_rule(
      cycle_a,
      cgpui::StyleClassRule{
          .reused_classes = cycle_a_reuse,
          .style = cgpui::StyleState{
              .base = cgpui::Style{}.with_z_index(4),
          },
      });
  cgpui::StyleClasses cycle_b_reuse;
  cycle_b_reuse.add(cycle_a);
  cascade.set_class_rule(
      cycle_b,
      cgpui::StyleClassRule{
          .reused_classes = cycle_b_reuse,
          .style = cgpui::StyleState{
              .base = cgpui::Style{}.with_layer(9),
          },
      });

  cgpui::StyleClasses classes;
  classes.add(card).add(cycle_a);
  const cgpui::Style resolved = cgpui::resolved_style(
      cascade,
      cgpui::StyleState{},
      classes,
      cgpui::StyleOverlay{},
      cgpui::StyleStateFlags{
          .hovered = true,
          .focused = true,
          .active = true,
      });

  if (!resolved.background_color.has_value() ||
      resolved.background_color->r != 12.0F / 255.0F ||
      !resolved.foreground_color.has_value() ||
      resolved.foreground_color->r != 50.0F / 255.0F) {
    return 95;
  }
  if (resolved.padding.left != 6.0F || resolved.border_width.left != 4.0F ||
      resolved.gap != 8.0F || resolved.opacity != 0.75F) {
    return 96;
  }
  if (resolved.z_index != 4 || resolved.layer != 9) {
    return 97;
  }

  const cgpui::StyleClassRule* card_rule = cascade.class_rule(card);
  const cgpui::StyleState* card_style = cascade.class_style(card);
  return card_rule != nullptr && card_style == &card_rule->style &&
                 card_rule->reused_classes.contains(compact)
             ? 0
             : 98;
}

int test_animation_easing_and_style_tween_primitives() {
  if (!same(cgpui::clamp_animation_progress(-0.5F), 0.0F) ||
      !same(cgpui::clamp_animation_progress(1.5F), 1.0F)) {
    return 82;
  }
  if (!same(cgpui::ease(cgpui::AnimationEasing::linear, 0.25F), 0.25F) ||
      !same(cgpui::ease(cgpui::AnimationEasing::ease_in, 0.5F), 0.25F) ||
      !same(cgpui::ease(cgpui::AnimationEasing::ease_out, 0.5F), 0.75F) ||
      !same(cgpui::ease(cgpui::AnimationEasing::ease_in_out, 0.25F), 0.125F) ||
      !same(cgpui::ease(cgpui::AnimationEasing::ease_in_out, 0.75F), 0.875F)) {
    return 83;
  }

  if (!same(cgpui::tween(10.0F, 20.0F, 0.25F), 12.5F)) {
    return 84;
  }
  const cgpui::Color mixed_color =
      cgpui::tween(cgpui::rgba(0, 0, 0, 0.25F),
                   cgpui::rgba(255, 128, 64, 0.75F),
                   0.5F);
  if (!same(mixed_color.r, 0.5F) ||
      !same(mixed_color.g, 64.0F / 255.0F) ||
      !same(mixed_color.b, 32.0F / 255.0F) ||
      !same(mixed_color.a, 0.5F)) {
    return 85;
  }

  const cgpui::Style start =
      cgpui::Style{}
          .with_background_color(cgpui::rgb(0, 0, 0))
          .with_opacity(0.2F)
          .with_transform(cgpui::AffineTransform::translation(0.0F, 10.0F));
  const cgpui::Style end =
      cgpui::Style{}
          .with_background_color(cgpui::rgb(100, 50, 0))
          .with_opacity(0.8F)
          .with_transform(cgpui::AffineTransform::translation(10.0F, 30.0F));
  const cgpui::StyleTween tween{
      .from = start,
      .to = end,
      .easing = cgpui::AnimationEasing::ease_out,
  };
  const cgpui::Style middle = tween.value_at(0.5F);
  if (!middle.background_color.has_value() ||
      !same(middle.background_color->r, 75.0F / 255.0F) ||
      !same(middle.opacity, 0.65F) ||
      !same_transform(
          middle.transform,
          cgpui::AffineTransform::translation(7.5F, 25.0F))) {
    return 86;
  }
  return 0;
}

} // namespace

static_assert(std::same_as<decltype(cgpui::EdgeSizes{}.top), float>);
static_assert(std::same_as<decltype(cgpui::BorderRadii{}.top_left), float>);
static_assert(std::same_as<decltype(cgpui::Style{}.overflow), cgpui::Overflow>);
static_assert(std::same_as<decltype(cgpui::Style{}.z_index), int>);
static_assert(std::same_as<decltype(cgpui::Style{}.layer), int>);
static_assert(std::same_as<decltype(cgpui::Style{}.gap), float>);
static_assert(std::same_as<decltype(cgpui::Style{}.flex_grow), float>);
static_assert(std::same_as<decltype(cgpui::Style{}.flex_shrink), float>);
static_assert(std::same_as<decltype(cgpui::Style{}.position), cgpui::Position>);
static_assert(std::same_as<decltype(cgpui::Style{}.inset), cgpui::EdgeSizes>);
static_assert(std::same_as<decltype(cgpui::FontDescriptor{}.family), std::string>);
static_assert(std::same_as<decltype(cgpui::Style{}.font), cgpui::FontDescriptor>);
static_assert(std::same_as<decltype(cgpui::Style{}.font_size), float>);
static_assert(std::same_as<decltype(cgpui::Style{}.opacity), float>);
static_assert(std::same_as<
              decltype(cgpui::Style{}.transform),
              cgpui::AffineTransform>);
static_assert(
    std::same_as<decltype(cgpui::Style{}.align_items), cgpui::AlignItems>);
static_assert(std::same_as<
              decltype(cgpui::Style{}.justify_content),
              cgpui::JustifyContent>);
static_assert(std::same_as<decltype(cgpui::Style{}.margin), cgpui::EdgeSizes>);
static_assert(std::same_as<decltype(cgpui::Style{}.min_size), cgpui::Size>);
static_assert(std::same_as<decltype(cgpui::Style{}.max_size), cgpui::Size>);
static_assert(std::same_as<
              decltype(cgpui::Style{}.percentage_size),
              cgpui::PercentageSize>);
static_assert(std::same_as<decltype(cgpui::px(1.0F)), float>);
static_assert(std::same_as<decltype(cgpui::rgb(255, 255, 255)), cgpui::Color>);
static_assert(
    std::same_as<decltype(cgpui::rgba(255, 255, 255, 1.0F)), cgpui::Color>);
static_assert(std::same_as<decltype(cgpui::edges(1.0F)), cgpui::EdgeSizes>);
static_assert(std::same_as<
              decltype(cgpui::StyleOverlay{}.preferred_size),
              std::optional<cgpui::Size>>);
static_assert(std::same_as<
              decltype(cgpui::StyleOverlay{}.min_size),
              std::optional<cgpui::Size>>);
static_assert(std::same_as<
              decltype(cgpui::StyleOverlay{}.max_size),
              std::optional<cgpui::Size>>);
static_assert(std::same_as<
              decltype(cgpui::StyleOverlay{}.percentage_size),
              cgpui::PercentageSize>);
static_assert(std::same_as<decltype(cgpui::StyleState{}.base), cgpui::Style>);
static_assert(std::same_as<
              decltype(cgpui::StyleState{}.active),
              cgpui::StyleOverlay>);
static_assert(std::same_as<
              decltype(cgpui::StyleStateFlags{}.hovered),
              bool>);
static_assert(std::same_as<
              decltype(cgpui::StyleStateFlags{}.active),
              bool>);
static_assert(std::equality_comparable<cgpui::StyleClassId>);
static_assert(std::equality_comparable<cgpui::ThemeTokenId>);
static_assert(std::same_as<decltype(cgpui::style_class("x")), cgpui::StyleClassId>);
static_assert(std::same_as<decltype(cgpui::theme_token("x")), cgpui::ThemeTokenId>);
static_assert(std::same_as<
              decltype(cgpui::StyleCascade{}.class_style(cgpui::style_class("x"))),
              const cgpui::StyleState*>);
static_assert(std::same_as<
              decltype(cgpui::StyleCascade{}.class_rule(cgpui::style_class("x"))),
              const cgpui::StyleClassRule*>);
static_assert(std::same_as<
              decltype(cgpui::StyleClassRule{}.reused_classes),
              cgpui::StyleClasses>);
static_assert(std::same_as<
              decltype(cgpui::StyleClassRule{}.style),
              cgpui::StyleState>);
static_assert(std::same_as<
              decltype(cgpui::ease(cgpui::AnimationEasing::linear, 0.0F)),
              float>);
static_assert(std::same_as<
              decltype(cgpui::StyleTween{}.value_at(0.5F)),
              cgpui::Style>);

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
  if (const int result =
          test_style_state_resolves_hover_focus_active_disabled_order();
      result != 0) {
    return result;
  }
  if (const int result = test_style_classes_store_named_class_ids();
      result != 0) {
    return result;
  }
  if (const int result = test_theme_color_and_spacing_tokens_lookup_softly();
      result != 0) {
    return result;
  }
  if (const int result =
          test_style_cascade_resolves_base_classes_state_and_inline_order();
      result != 0) {
    return result;
  }
  if (const int result =
          test_style_cascade_reuses_class_rules_depth_first_and_ignores_cycles();
      result != 0) {
    return result;
  }
  if (const int result = test_animation_easing_and_style_tween_primitives();
      result != 0) {
    return result;
  }
  return 0;
}
