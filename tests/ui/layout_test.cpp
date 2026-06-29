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
