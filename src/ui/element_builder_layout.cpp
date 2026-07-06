#include "cgpui/ui/element_builder_core.hpp"

#include <utility>

namespace cgpui {

ElementBuilder ElementBuilder::size(Size size) && {
  style_state_.base = style_state_.base.with_preferred_size(size);
  if (kind_ == Kind::fixed_size || kind_ == Kind::child_view) {
    size_ = size;
  }
  return std::move(*this);
}

ElementBuilder ElementBuilder::size(float width, float height) && {
  return std::move(*this).size(Size{.width = width, .height = height});
}

ElementBuilder ElementBuilder::w(float width) && {
  Size size = style_state_.base.preferred_size;
  if (kind_ == Kind::fixed_size || kind_ == Kind::child_view) {
    size = size_;
  }
  size.width = width;
  return std::move(*this).size(size);
}

ElementBuilder ElementBuilder::h(float height) && {
  Size size = style_state_.base.preferred_size;
  if (kind_ == Kind::fixed_size || kind_ == Kind::child_view) {
    size = size_;
  }
  size.height = height;
  return std::move(*this).size(size);
}

ElementBuilder ElementBuilder::min_size(Size size) && {
  style_state_.base = style_state_.base.with_min_size(size);
  return std::move(*this);
}

ElementBuilder ElementBuilder::max_size(Size size) && {
  style_state_.base = style_state_.base.with_max_size(size);
  return std::move(*this);
}

ElementBuilder ElementBuilder::min_w(float width) && {
  Size size = style_state_.base.min_size;
  size.width = width;
  return std::move(*this).min_size(size);
}

ElementBuilder ElementBuilder::min_h(float height) && {
  Size size = style_state_.base.min_size;
  size.height = height;
  return std::move(*this).min_size(size);
}

ElementBuilder ElementBuilder::max_w(float width) && {
  Size size = style_state_.base.max_size;
  size.width = width;
  return std::move(*this).max_size(size);
}

ElementBuilder ElementBuilder::max_h(float height) && {
  Size size = style_state_.base.max_size;
  size.height = height;
  return std::move(*this).max_size(size);
}

ElementBuilder ElementBuilder::size_pct(
    float width_percent,
    float height_percent) && {
  style_state_.base = style_state_.base.with_percentage_size(
      PercentageSize{.width = width_percent, .height = height_percent});
  return std::move(*this);
}

ElementBuilder ElementBuilder::w_pct(float percent) && {
  style_state_.base = style_state_.base.with_width_percent(percent);
  return std::move(*this);
}

ElementBuilder ElementBuilder::h_pct(float percent) && {
  style_state_.base = style_state_.base.with_height_percent(percent);
  return std::move(*this);
}

ElementBuilder ElementBuilder::padding(EdgeSizes edges) && {
  style_state_.base = style_state_.base.with_padding(edges);
  return std::move(*this);
}

ElementBuilder ElementBuilder::p(float value) && {
  return std::move(*this).padding(edges(value));
}

ElementBuilder ElementBuilder::px(float value) && {
  EdgeSizes edge_sizes = style_state_.base.padding;
  edge_sizes.left = value;
  edge_sizes.right = value;
  return std::move(*this).padding(edge_sizes);
}

ElementBuilder ElementBuilder::py(float value) && {
  EdgeSizes edge_sizes = style_state_.base.padding;
  edge_sizes.top = value;
  edge_sizes.bottom = value;
  return std::move(*this).padding(edge_sizes);
}

ElementBuilder ElementBuilder::pt(float value) && {
  EdgeSizes edge_sizes = style_state_.base.padding;
  edge_sizes.top = value;
  return std::move(*this).padding(edge_sizes);
}

ElementBuilder ElementBuilder::pr(float value) && {
  EdgeSizes edge_sizes = style_state_.base.padding;
  edge_sizes.right = value;
  return std::move(*this).padding(edge_sizes);
}

ElementBuilder ElementBuilder::pb(float value) && {
  EdgeSizes edge_sizes = style_state_.base.padding;
  edge_sizes.bottom = value;
  return std::move(*this).padding(edge_sizes);
}

ElementBuilder ElementBuilder::pl(float value) && {
  EdgeSizes edge_sizes = style_state_.base.padding;
  edge_sizes.left = value;
  return std::move(*this).padding(edge_sizes);
}

ElementBuilder ElementBuilder::margin(EdgeSizes edges) && {
  style_state_.base = style_state_.base.with_margin(edges);
  return std::move(*this);
}

ElementBuilder ElementBuilder::m(float value) && {
  return std::move(*this).margin(edges(value));
}

ElementBuilder ElementBuilder::mx(float value) && {
  EdgeSizes edge_sizes = style_state_.base.margin;
  edge_sizes.left = value;
  edge_sizes.right = value;
  return std::move(*this).margin(edge_sizes);
}

ElementBuilder ElementBuilder::my(float value) && {
  EdgeSizes edge_sizes = style_state_.base.margin;
  edge_sizes.top = value;
  edge_sizes.bottom = value;
  return std::move(*this).margin(edge_sizes);
}

ElementBuilder ElementBuilder::mt(float value) && {
  EdgeSizes edge_sizes = style_state_.base.margin;
  edge_sizes.top = value;
  return std::move(*this).margin(edge_sizes);
}

ElementBuilder ElementBuilder::mr(float value) && {
  EdgeSizes edge_sizes = style_state_.base.margin;
  edge_sizes.right = value;
  return std::move(*this).margin(edge_sizes);
}

ElementBuilder ElementBuilder::mb(float value) && {
  EdgeSizes edge_sizes = style_state_.base.margin;
  edge_sizes.bottom = value;
  return std::move(*this).margin(edge_sizes);
}

ElementBuilder ElementBuilder::ml(float value) && {
  EdgeSizes edge_sizes = style_state_.base.margin;
  edge_sizes.left = value;
  return std::move(*this).margin(edge_sizes);
}

ElementBuilder ElementBuilder::border_width(EdgeSizes edges) && {
  style_state_.base = style_state_.base.with_border_width(edges);
  return std::move(*this);
}

ElementBuilder ElementBuilder::border_1() && {
  return std::move(*this).border_width(edges(1.0F));
}

ElementBuilder ElementBuilder::gap(float value) && {
  style_state_.base = style_state_.base.with_gap(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::align_items(AlignItems value) && {
  style_state_.base = style_state_.base.with_align_items(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::items_start() && {
  return std::move(*this).align_items(AlignItems::start);
}

ElementBuilder ElementBuilder::items_center() && {
  return std::move(*this).align_items(AlignItems::center);
}

ElementBuilder ElementBuilder::items_end() && {
  return std::move(*this).align_items(AlignItems::end);
}

ElementBuilder ElementBuilder::justify_content(JustifyContent value) && {
  style_state_.base = style_state_.base.with_justify_content(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::justify_start() && {
  return std::move(*this).justify_content(JustifyContent::start);
}

ElementBuilder ElementBuilder::justify_center() && {
  return std::move(*this).justify_content(JustifyContent::center);
}

ElementBuilder ElementBuilder::justify_end() && {
  return std::move(*this).justify_content(JustifyContent::end);
}

ElementBuilder ElementBuilder::justify_between() && {
  return std::move(*this).justify_content(JustifyContent::space_between);
}

ElementBuilder ElementBuilder::flex_grow(float value) && {
  style_state_.base = style_state_.base.with_flex_grow(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::flex_shrink(float value) && {
  style_state_.base = style_state_.base.with_flex_shrink(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::flex_1() && {
  return std::move(*this).flex_grow(1.0F).flex_shrink(1.0F);
}

ElementBuilder ElementBuilder::layer(int value) && {
  style_state_.base = style_state_.base.with_layer(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::z_index(int value) && {
  style_state_.base = style_state_.base.with_z_index(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::position(Position value) && {
  style_state_.base = style_state_.base.with_position(value);
  return std::move(*this);
}

ElementBuilder ElementBuilder::absolute() && {
  return std::move(*this).position(Position::absolute);
}

ElementBuilder ElementBuilder::fixed() && {
  return std::move(*this).position(Position::fixed);
}

ElementBuilder ElementBuilder::relative() && {
  return std::move(*this).position(Position::relative);
}

ElementBuilder ElementBuilder::inset(EdgeSizes edges) && {
  style_state_.base = style_state_.base.with_inset(edges);
  return std::move(*this);
}

ElementBuilder ElementBuilder::top(float value) && {
  EdgeSizes inset = style_state_.base.inset;
  inset.top = value;
  return std::move(*this).inset(inset);
}

ElementBuilder ElementBuilder::right(float value) && {
  EdgeSizes inset = style_state_.base.inset;
  inset.right = value;
  return std::move(*this).inset(inset);
}

ElementBuilder ElementBuilder::bottom(float value) && {
  EdgeSizes inset = style_state_.base.inset;
  inset.bottom = value;
  return std::move(*this).inset(inset);
}

ElementBuilder ElementBuilder::left(float value) && {
  EdgeSizes inset = style_state_.base.inset;
  inset.left = value;
  return std::move(*this).inset(inset);
}

} // namespace cgpui
