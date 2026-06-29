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

int test_style_defaults_are_empty() {
  const cgpui::Style style;
  if (style.background_color.has_value() || style.foreground_color.has_value()) {
    return 5;
  }
  if (!same(style.preferred_size.width, 0.0F) ||
      !same(style.preferred_size.height, 0.0F)) {
    return 6;
  }
  if (!same(style.padding.top, 0.0F) ||
      !same(style.border_width.left, 0.0F)) {
    return 7;
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
          .with_border_width(cgpui::EdgeSizes::all(2.0F));

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
  if (!same(style.border_width.right, 2.0F) ||
      !same(style.border_width.bottom, 2.0F)) {
    return 12;
  }

  return 0;
}

} // namespace

static_assert(std::same_as<decltype(cgpui::EdgeSizes{}.top), float>);

int main() {
  if (const int result = test_edge_sizes_default_to_zero(); result != 0) {
    return result;
  }
  if (const int result = test_edge_sizes_helpers_expand_values();
      result != 0) {
    return result;
  }
  if (const int result = test_style_defaults_are_empty(); result != 0) {
    return result;
  }
  if (const int result = test_style_builder_methods_store_values();
      result != 0) {
    return result;
  }
  return 0;
}
