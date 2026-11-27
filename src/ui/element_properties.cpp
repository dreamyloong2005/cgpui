#include "cgpui/ui/element_core.hpp"

#include <utility>

namespace cgpui {

ElementId Element::id() const {
  return id_;
}

const std::optional<ElementKey>& Element::key() const {
  return key_;
}

bool Element::enabled() const {
  return enabled_;
}

void Element::set_enabled(bool enabled) {
  enabled_ = enabled;
}

float Element::flex_grow() const {
  return flex_grow_;
}

void Element::set_flex_grow(float value) {
  flex_grow_ = value;
}

float Element::flex_shrink() const {
  return flex_shrink_;
}

void Element::set_flex_shrink(float value) {
  flex_shrink_ = value;
}

Position Element::position() const {
  return position_;
}

void Element::set_position(Position value) {
  position_ = value;
}

EdgeSizes Element::inset() const {
  return inset_;
}

void Element::set_inset(EdgeSizes value) {
  inset_ = value;
}

int Element::z_index() const {
  return z_index_;
}

void Element::set_z_index(int value) {
  z_index_ = value;
}

int Element::layer() const {
  return layer_;
}

void Element::set_layer(int value) {
  layer_ = value;
}

int Element::z_order() const {
  const int explicit_z_index = z_index();
  return explicit_z_index != 0 ? explicit_z_index : layer();
}

void Element::assign_id(ElementId id) {
  id_ = id;
}

void Element::set_key(std::optional<ElementKey> key) {
  key_ = std::move(key);
}

} // namespace cgpui
