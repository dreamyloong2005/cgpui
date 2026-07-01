#include "cgpui/ui/layout.hpp"

#include <cmath>

namespace {

bool same(float lhs, float rhs) {
  return lhs == rhs;
}

int test_layout_input_defaults_to_zero_min_and_unbounded_max() {
  const cgpui::LayoutInput input;

  if (!same(input.constraints.min_size.width, 0.0F) ||
      !same(input.constraints.min_size.height, 0.0F)) {
    return 1;
  }
  if (!std::isinf(input.constraints.max_size.width) ||
      !std::isinf(input.constraints.max_size.height)) {
    return 2;
  }

  return 0;
}

int test_layout_input_stores_constraints() {
  const cgpui::LayoutInput input{
      .constraints =
          {
              .min_size = {.width = 10.0F, .height = 20.0F},
              .max_size = {.width = 100.0F, .height = 200.0F},
          },
  };

  if (!same(input.constraints.min_size.width, 10.0F) ||
      !same(input.constraints.min_size.height, 20.0F) ||
      !same(input.constraints.max_size.width, 100.0F) ||
      !same(input.constraints.max_size.height, 200.0F)) {
    return 3;
  }

  return 0;
}

int test_layout_input_carries_scale_and_converts_logical_device_pixels() {
  const cgpui::LayoutInput input{
      .constraints =
          {
              .min_size = {.width = 10.0F, .height = 20.0F},
              .max_size = {.width = 100.0F, .height = 200.0F},
          },
      .scale = cgpui::DpiScale{2.0F},
  };

  if (!same(input.scale.value, 2.0F)) {
    return 8;
  }

  const cgpui::Size logical = cgpui::to_logical_pixels(
      cgpui::Size{.width = 320.0F, .height = 240.0F},
      input.scale);
  if (!same(logical.width, 160.0F) || !same(logical.height, 120.0F)) {
    return 9;
  }

  const cgpui::Size device = cgpui::to_device_pixels(
      cgpui::Size{.width = 160.0F, .height = 120.0F},
      input.scale);
  if (!same(device.width, 320.0F) || !same(device.height, 240.0F)) {
    return 10;
  }

  const cgpui::Rect device_rect = cgpui::to_device_pixels(
      cgpui::Rect{
          .origin = {.x = 4.0F, .y = 6.0F},
          .size = {.width = 8.0F, .height = 10.0F},
      },
      input.scale);
  return same(device_rect.origin.x, 8.0F) &&
                 same(device_rect.origin.y, 12.0F) &&
                 same(device_rect.size.width, 16.0F) &&
                 same(device_rect.size.height, 20.0F)
             ? 0
             : 11;
}

int test_constrain_size_clamps_each_axis_between_min_and_max() {
  const cgpui::LayoutConstraints constraints{
      .min_size = {.width = 10.0F, .height = 20.0F},
      .max_size = {.width = 100.0F, .height = 120.0F},
  };

  const cgpui::Size below =
      cgpui::constrain_size(cgpui::Size{.width = 5.0F, .height = 15.0F},
                            constraints);
  if (!same(below.width, 10.0F) || !same(below.height, 20.0F)) {
    return 4;
  }

  const cgpui::Size inside =
      cgpui::constrain_size(cgpui::Size{.width = 50.0F, .height = 60.0F},
                            constraints);
  if (!same(inside.width, 50.0F) || !same(inside.height, 60.0F)) {
    return 5;
  }

  const cgpui::Size above =
      cgpui::constrain_size(cgpui::Size{.width = 150.0F, .height = 160.0F},
                            constraints);
  if (!same(above.width, 100.0F) || !same(above.height, 120.0F)) {
    return 6;
  }

  return 0;
}

int test_layout_output_stores_origin_and_size() {
  const cgpui::LayoutOutput output{
      .origin = {.x = 3.0F, .y = 4.0F},
      .size = {.width = 42.0F, .height = 24.0F},
  };

  if (!same(output.origin.x, 3.0F) || !same(output.origin.y, 4.0F) ||
      !same(output.size.width, 42.0F) ||
      !same(output.size.height, 24.0F)) {
    return 7;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result =
          test_layout_input_defaults_to_zero_min_and_unbounded_max();
      result != 0) {
    return result;
  }
  if (const int result = test_layout_input_stores_constraints(); result != 0) {
    return result;
  }
  if (const int result =
          test_layout_input_carries_scale_and_converts_logical_device_pixels();
      result != 0) {
    return result;
  }
  if (const int result =
          test_constrain_size_clamps_each_axis_between_min_and_max();
      result != 0) {
    return result;
  }
  if (const int result = test_layout_output_stores_origin_and_size();
      result != 0) {
    return result;
  }
  return 0;
}
