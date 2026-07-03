#include "ui_internal.hpp"

namespace cgpui {

WindowOptions& WindowOptions::title(std::string title_value) {
  descriptor.title = std::move(title_value);
  return *this;
}

WindowOptions& WindowOptions::size(Size size_value) {
  descriptor.size = size_value;
  return *this;
}

WindowOptions& WindowOptions::size(float width, float height) {
  descriptor.size = Size{.width = width, .height = height};
  return *this;
}

WindowOptions& WindowOptions::titlebar_visible(bool visible) {
  descriptor.chrome.titlebar_visible = visible;
  return *this;
}

WindowOptions& WindowOptions::decorations(bool enabled) {
  descriptor.chrome.decorations = enabled;
  return *this;
}

WindowOptions& WindowOptions::resizable(bool enabled) {
  descriptor.chrome.resizable = enabled;
  return *this;
}

WindowOptions& WindowOptions::transparent(bool enabled) {
  descriptor.chrome.transparent_background = enabled;
  return *this;
}

WindowDescriptor WindowOptions::to_descriptor() const {
  return descriptor;
}

} // namespace cgpui
