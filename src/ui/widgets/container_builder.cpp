#include "cgpui/ui/container_builder.hpp"

namespace cgpui {

ElementBuilder div() {
  return ElementBuilder::box();
}

ElementBuilder h_flex() {
  return ElementBuilder::row();
}

ElementBuilder v_flex() {
  return ElementBuilder::column();
}

ElementBuilder h_stack() {
  return ElementBuilder::row();
}

ElementBuilder v_stack() {
  return ElementBuilder::v_stack();
}

} // namespace cgpui
