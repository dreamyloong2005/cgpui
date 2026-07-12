#include "cgpui/prelude.hpp"

#include <cmath>

namespace {

bool near(float left, float right) {
  return std::fabs(left - right) < 0.0001F;
}

bool near(cgpui::Size value, cgpui::Size expected) {
  return near(value.width, expected.width) &&
         near(value.height, expected.height);
}

bool near(cgpui::EdgeSizes value, cgpui::EdgeSizes expected) {
  return near(value.top, expected.top) &&
         near(value.right, expected.right) &&
         near(value.bottom, expected.bottom) &&
         near(value.left, expected.left);
}

int test_layout_and_box_interpolation() {
  cgpui::Style from;
  from.preferred_size = {10.0F, 20.0F};
  from.min_size = {2.0F, 4.0F};
  from.max_size = {100.0F, 200.0F};
  from.percentage_size = {.width = 0.2F, .height = 0.4F};
  from.padding = cgpui::edges(2.0F, 4.0F, 6.0F, 8.0F);
  from.margin = cgpui::edges(1.0F, 3.0F, 5.0F, 7.0F);
  from.border_width = cgpui::edges(1.0F);
  from.border_radius = cgpui::BorderRadii::corners(2.0F, 4.0F, 6.0F, 8.0F);
  from.gap = 4.0F;
  from.flex_grow = 1.0F;
  from.flex_shrink = 3.0F;
  from.inset = cgpui::edges(0.0F, 10.0F, 20.0F, 30.0F);
  from.font_size = 12.0F;

  cgpui::Style to = from;
  to.preferred_size = {30.0F, 60.0F};
  to.min_size = {6.0F, 12.0F};
  to.max_size = {200.0F, 400.0F};
  to.percentage_size = {.width = 0.6F, .height = 0.8F};
  to.padding = cgpui::edges(6.0F, 8.0F, 10.0F, 12.0F);
  to.margin = cgpui::edges(5.0F, 7.0F, 9.0F, 11.0F);
  to.border_width = cgpui::edges(5.0F);
  to.border_radius = cgpui::BorderRadii::corners(6.0F, 8.0F, 10.0F, 12.0F);
  to.gap = 12.0F;
  to.flex_grow = 5.0F;
  to.flex_shrink = 7.0F;
  to.inset = cgpui::edges(4.0F, 14.0F, 24.0F, 34.0F);
  to.font_size = 20.0F;

  const cgpui::Style value = cgpui::tween(from, to, 0.25F);
  if (!near(value.preferred_size, {15.0F, 30.0F}) ||
      !near(value.min_size, {3.0F, 6.0F}) ||
      !near(value.max_size, {125.0F, 250.0F}) ||
      !value.percentage_size.width.has_value() ||
      !value.percentage_size.height.has_value() ||
      !near(*value.percentage_size.width, 0.3F) ||
      !near(*value.percentage_size.height, 0.5F) ||
      !near(value.padding, cgpui::edges(3.0F, 5.0F, 7.0F, 9.0F)) ||
      !near(value.margin, cgpui::edges(2.0F, 4.0F, 6.0F, 8.0F)) ||
      !near(value.border_width, cgpui::edges(2.0F)) ||
      !near(value.border_radius.top_left, 3.0F) ||
      !near(value.border_radius.bottom_left, 9.0F) ||
      !near(value.gap, 6.0F) || !near(value.flex_grow, 2.0F) ||
      !near(value.flex_shrink, 4.0F) ||
      !near(value.inset, cgpui::edges(1.0F, 11.0F, 21.0F, 31.0F)) ||
      !near(value.font_size, 14.0F)) return 1;
  return 0;
}

int test_paint_geometry_interpolation() {
  cgpui::Style from;
  from.background_color = cgpui::rgba(0, 20, 40, 0.2F);
  from.foreground_color = cgpui::rgba(10, 30, 50, 0.4F);
  from.border_color = cgpui::rgba(20, 40, 60, 0.6F);
  from.box_shadow = cgpui::BoxShadow{
      .color = cgpui::rgba(0, 0, 0, 0.2F),
      .offset = {2.0F, 4.0F},
      .blur_radius = 6.0F,
      .spread_radius = 8.0F,
  };
  from.clip_rect = cgpui::Rect{.origin = {2.0F, 4.0F}, .size = {20.0F, 40.0F}};
  from.opacity = 0.2F;
  from.transform = cgpui::AffineTransform::translation(10.0F, 20.0F);

  cgpui::Style to = from;
  to.background_color = cgpui::rgba(100, 120, 140, 0.6F);
  to.foreground_color = cgpui::rgba(110, 130, 150, 0.8F);
  to.border_color = cgpui::rgba(120, 140, 160, 1.0F);
  to.box_shadow = cgpui::BoxShadow{
      .color = cgpui::rgba(100, 100, 100, 0.6F),
      .offset = {6.0F, 8.0F},
      .blur_radius = 10.0F,
      .spread_radius = 12.0F,
  };
  to.clip_rect = cgpui::Rect{.origin = {6.0F, 8.0F}, .size = {60.0F, 80.0F}};
  to.opacity = 0.6F;
  to.transform = cgpui::AffineTransform::translation(30.0F, 40.0F);

  const cgpui::Style value = cgpui::tween(from, to, 0.5F);
  if (!value.background_color.has_value() ||
      !near(value.background_color->r, 50.0F / 255.0F) ||
      !value.box_shadow.has_value() ||
      !near(value.box_shadow->color.r, 50.0F / 255.0F) ||
      !near(value.box_shadow->offset.x, 4.0F) ||
      !near(value.box_shadow->offset.y, 6.0F) ||
      !near(value.box_shadow->blur_radius, 8.0F) ||
      !near(value.box_shadow->spread_radius, 10.0F) ||
      !value.clip_rect.has_value() ||
      !near(value.clip_rect->origin.x, 4.0F) ||
      !near(value.clip_rect->origin.y, 6.0F) ||
      !near(value.clip_rect->size, {40.0F, 60.0F}) ||
      !near(value.opacity, 0.4F) ||
      !near(value.transform.translate_x, 20.0F) ||
      !near(value.transform.translate_y, 30.0F)) return 2;
  return 0;
}

int test_discrete_optional_and_non_finite_policy() {
  cgpui::Style from;
  from.overflow = cgpui::Overflow::visible;
  from.align_items = cgpui::AlignItems::start;
  from.position = cgpui::Position::relative;
  from.z_index = 2;
  from.background_color = cgpui::rgb(1, 2, 3);

  cgpui::Style to;
  to.overflow = cgpui::Overflow::hidden;
  to.align_items = cgpui::AlignItems::end;
  to.position = cgpui::Position::absolute;
  to.z_index = 8;

  const cgpui::Style middle = cgpui::tween(from, to, 0.5F);
  const cgpui::Style end = cgpui::tween(from, to, 1.0F);
  if (middle.overflow != cgpui::Overflow::visible ||
      middle.align_items != cgpui::AlignItems::start ||
      middle.position != cgpui::Position::relative || middle.z_index != 2 ||
      !middle.background_color.has_value() ||
      end.overflow != cgpui::Overflow::hidden ||
      end.align_items != cgpui::AlignItems::end ||
      end.position != cgpui::Position::absolute || end.z_index != 8 ||
      end.background_color.has_value() ||
      !std::isinf(middle.max_size.width) ||
      !std::isinf(middle.max_size.height)) return 3;
  return 0;
}

} // namespace

int main() {
  if (const int result = test_layout_and_box_interpolation()) return result;
  if (const int result = test_paint_geometry_interpolation()) return result;
  return test_discrete_optional_and_non_finite_policy();
}
