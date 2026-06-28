#include "cgpui/core/error.hpp"
#include "cgpui/core/events.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/core/window.hpp"

int main() {
  cgpui::WindowDescriptor descriptor;
  cgpui::WindowState state;
  state.framebuffer_size = descriptor.size;
  return state.framebuffer_size.width > 0.0F ? 0 : 1;
}
