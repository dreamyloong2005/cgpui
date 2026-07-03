#include "cgpui/ui/element_builder_core.hpp"

namespace cgpui {

ElementBuilder ElementBuilder::box() {
  return ElementBuilder(Kind::box);
}

ElementBuilder ElementBuilder::row() {
  return ElementBuilder(Kind::row);
}

ElementBuilder ElementBuilder::column() {
  return ElementBuilder(Kind::column);
}

ElementBuilder ElementBuilder::v_stack() {
  return ElementBuilder(Kind::v_stack);
}

ElementBuilder ElementBuilder::fixed_size(Size size) {
  ElementBuilder builder(Kind::fixed_size);
  builder.size_ = size;
  return builder;
}

ElementBuilder ElementBuilder::text(TextModel& model) {
  ElementBuilder builder(Kind::text);
  builder.text_model_ = &model;
  return builder;
}

ElementBuilder ElementBuilder::child_view(ViewId view_id) {
  ElementBuilder builder(Kind::child_view);
  builder.child_view_id_ = view_id;
  return builder;
}

AnyElement into_element(AnyElement element) {
  return element;
}

AnyElement into_element(ElementBuilder builder) {
  return std::move(builder).build();
}

ElementBuilder div() {
  return ElementBuilder::box();
}

ElementBuilder h_flex() {
  return ElementBuilder::row();
}

ElementBuilder v_flex() {
  return ElementBuilder::column();
}

ElementBuilder v_stack() {
  return ElementBuilder::v_stack();
}

ElementBuilder text(TextModel& model) {
  return ElementBuilder::text(model);
}

ElementBuilder child_view(ViewId view_id) {
  return ElementBuilder::child_view(view_id);
}

} // namespace cgpui
