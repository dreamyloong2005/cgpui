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
  const cgpui::Entity<int> entity_id = entities.insert(1);
  const cgpui::Model<int> model_id = entity_id;
  const cgpui::WeakEntity<int> weak_entity(model_id);
  (void)weak_entity;
  (void)entity_id;
  (void)model_id;

  cgpui::MemoryClipboard clipboard;
  (void)clipboard.write_text("x");

  cgpui::GlyphCache glyph_cache;
  const cgpui::GlyphAtlasKey glyph_key{
      .font_family = "Header",
      .font_size = 18.0F,
      .glyph_index = 0,
      .byte_offset = 0,
      .byte_length = 1,
  };
  const cgpui::GlyphCacheLookup glyph_lookup = glyph_cache.lookup(glyph_key);
  glyph_cache.store(cgpui::GlyphAtlasEntry{
      .key = glyph_key,
      .atlas_bounds = cgpui::Rect{.size = {9.0F, 18.0F}},
      .advance = 9.0F,
  });

  cgpui::Win32SurfaceHandle win32_surface;
  cgpui::NativeSurfaceHandle surface = win32_surface;
  (void)surface;

  return state.framebuffer_size.width > 0.0F && !glyph_lookup.hit &&
                 glyph_cache.lookup_count() == 1 &&
                 glyph_cache.entries().size() == 1
             ? 0
             : 1;
}
