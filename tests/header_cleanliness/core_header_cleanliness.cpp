#include "cgpui/core/error.hpp"
#include "cgpui/core/events.hpp"
#include "cgpui/core/entity.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/core/window.hpp"
#include "cgpui/platform/clipboard.hpp"
#include "cgpui/platform/native_surface.hpp"
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

int main() {
  cgpui::WindowDescriptor descriptor;
  cgpui::WindowState state;
  state.framebuffer_size = descriptor.size;
  cgpui::PlatformEvent event = cgpui::WindowRedrawRequested{};
  (void)event;
  cgpui::PlatformEvent ime_event = cgpui::ImeComposition{
      .phase = cgpui::ImeCompositionPhase::update,
      .text = "x"};
  (void)ime_event;

  cgpui::EntityStore<int> entities;
  const auto entity_id = entities.insert(1);
  (void)entity_id;

  cgpui::MemoryClipboard clipboard;
  (void)clipboard.write_text("x");

  cgpui::Win32SurfaceHandle win32_surface;
  cgpui::NativeSurfaceHandle surface = win32_surface;
  (void)surface;

  return state.framebuffer_size.width > 0.0F ? 0 : 1;
}
