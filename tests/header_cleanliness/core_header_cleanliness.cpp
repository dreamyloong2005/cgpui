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
  state.ime_text_input_support = cgpui::ImeTextInputSupport::unsupported;
  cgpui::PlatformEvent event = cgpui::WindowRedrawRequested{};
  (void)event;
  cgpui::PlatformEvent wakeup_event = cgpui::WindowWakeupRequested{};
  (void)wakeup_event;
  cgpui::PlatformEvent ime_event = cgpui::ImeComposition{
      .phase = cgpui::ImeCompositionPhase::update,
      .text = "x"};
  (void)ime_event;
  cgpui::DragDropPayload drag_payload{
      .kind = cgpui::DragDropPayloadKind::files,
      .text = {},
      .files = {"C:\\Temp\\first.txt"},
  };
  cgpui::PlatformEvent drag_event = cgpui::DragEntered{
      .position = {1.0F, 2.0F},
      .payload = drag_payload};
  (void)drag_event;

  cgpui::EntityStore<int> entities;
  const cgpui::Entity<int> entity_id = entities.insert(1);
  const cgpui::Model<int> model_id = entity_id;
  const cgpui::WeakEntity<int> weak_entity(model_id);
  (void)weak_entity;
  (void)entity_id;
  (void)model_id;

  cgpui::MemoryClipboard clipboard;
  (void)clipboard.write_text("x");

  const cgpui::AffineTransform transform = cgpui::compose(
      cgpui::AffineTransform::translation(1.0F, 2.0F),
      cgpui::AffineTransform::translation(3.0F, 4.0F));
  const cgpui::PaintMetadata paint_metadata{
      .opacity = 0.5F,
      .transform = transform,
  };

  cgpui::GlyphCache glyph_cache;
  const cgpui::GlyphAtlasKey glyph_key{
      .font_family = "Header",
      .font_size = 18.0F,
      .glyph_index = 0,
      .byte_offset = 0,
      .byte_length = 1,
  };
  const cgpui::GlyphCacheLookup glyph_lookup = glyph_cache.lookup(glyph_key);
  const cgpui::TextGlyphPaint glyph_paint{
      .key = glyph_key,
      .origin = {},
      .advance = 9.0F,
      .device_origin = {},
      .device_advance = 9.0F,
  };
  const cgpui::RasterizedGlyph rasterized =
      cgpui::rasterize_fallback_glyph(glyph_paint);
  const cgpui::GlyphAtlasAllocation allocation =
      glyph_cache.allocate(rasterized);
  const std::span<const cgpui::GlyphUploadRecord> upload_records =
      glyph_cache.upload_records();
  const std::span<const cgpui::GlyphAtlasPage> atlas_pages =
      glyph_cache.atlas_pages();
  const cgpui::GlyphAtlasImageDescriptor atlas_image{
      .page_index = allocation.page_index,
      .size = atlas_pages[0].size,
      .format = cgpui::GlyphAtlasImageFormat::alpha8_unorm,
      .upload_count = upload_records.size(),
  };
  const cgpui::GlyphAtlasUploadRegion upload_region{
      .key = upload_records[0].key,
      .page_index = upload_records[0].page_index,
      .atlas_bounds = upload_records[0].atlas_bounds,
      .width = upload_records[0].width,
      .height = upload_records[0].height,
      .stride = upload_records[0].stride,
      .byte_size = upload_records[0].alpha.size(),
  };
  const cgpui::GlyphAtlasUploadBatch upload_batch{
      .image = atlas_image,
      .uploads = {upload_region},
      .alpha = upload_records[0].alpha,
  };
  glyph_cache.store(cgpui::GlyphAtlasEntry{
      .key = glyph_key,
      .atlas_bounds = cgpui::Rect{.size = {9.0F, 18.0F}},
      .advance = 9.0F,
  });
  const cgpui::TextDraw text_draw{
      .bounds = cgpui::Rect{.size = {9.0F, 18.0F}},
      .color = cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      .font = cgpui::FontDescriptor{.family = "Header"},
      .content = "x",
      .byte_length = 1,
      .font_size = 18.0F,
      .glyphs =
          {
              cgpui::TextGlyphPaint{
                  .key = glyph_key,
                  .origin = {},
                  .advance = 9.0F,
              },
      },
      .clip_rect = cgpui::Rect{.size = {9.0F, 18.0F}},
      .metadata = paint_metadata,
  };
  const cgpui::TexturedGlyphQuad textured_quad{
      .key = glyph_key,
      .page_index = allocation.page_index,
      .device_bounds = cgpui::Rect{.size = {9.0F, 18.0F}},
      .atlas_bounds = allocation.atlas_bounds,
      .atlas_uv_bounds = cgpui::Rect{.size = {9.0F / 256.0F, 18.0F / 256.0F}},
      .color = text_draw.color,
      .clip_rect = text_draw.clip_rect,
      .metadata = paint_metadata,
  };
  const cgpui::RendererCommandBatch batch{
      .key =
          cgpui::RendererCommandBatchKey{
              .primitive_kind = cgpui::RendererPrimitiveKind::text,
              .clip_rect = cgpui::Rect{.size = {9.0F, 18.0F}},
              .metadata = paint_metadata,
          },
      .command_count = 1,
      .command_indices = {0},
  };
  const cgpui::RendererCommandStreamItem unsupported_command{
      .primitive_kind = cgpui::RendererPrimitiveKind::text_caret,
      .command_index = 7,
      .clip_rect = cgpui::Rect{.size = {2.0F, 18.0F}},
      .metadata = paint_metadata,
  };
  cgpui::RendererCommandReport report{
      .batches = {batch},
      .unsupported_commands =
          {
              cgpui::RendererUnsupportedCommandDiagnostic{
                  .primitive_kind = unsupported_command.primitive_kind,
                  .command_index = unsupported_command.command_index,
                  .reason = cgpui::RendererUnsupportedCommandReason::
                      unsupported_primitive,
                  .message = std::string(
                      cgpui::renderer_primitive_kind_name(
                          unsupported_command.primitive_kind)),
              },
          },
      .supported_command_count = 1,
      .unsupported_command_count = 1,
  };
  cgpui::PlatformAccessibilityTreeUpdate accessibility_update{
      .root_element_id = 2,
      .node_count = 3,
      .focused_node_count = 1,
      .nodes =
          {
              cgpui::PlatformAccessibilityNodeUpdate{
                  .element_id = 2,
                  .role = cgpui::PlatformAccessibilityRole::text_input,
                  .name = "Input",
                  .text = "value",
                  .focusable = true,
                  .focused = true,
                  .child_count = 0,
              },
          },
  };

  cgpui::Win32SurfaceHandle win32_surface;
  cgpui::NativeSurfaceHandle surface = win32_surface;
  (void)surface;

  return state.framebuffer_size.width > 0.0F && !glyph_lookup.hit &&
                 glyph_cache.lookup_count() == 1 &&
                 glyph_cache.entries().size() == 1 &&
                 !rasterized.bitmap.empty() &&
                 rasterized.bitmap.width == 9 &&
                 allocation.created &&
                 atlas_image.upload_count == 1 &&
                 upload_records.size() == 1 &&
                 upload_region.byte_size == upload_records[0].alpha.size() &&
                 upload_batch.uploads.size() == 1 &&
                 atlas_pages.size() == 1 &&
                 textured_quad.page_index == 0 &&
                 textured_quad.atlas_uv_bounds.size.width > 0.0F &&
                 text_draw.glyphs.size() == 1 &&
                 text_draw.metadata.transform.translate_x == 4.0F &&
                 text_draw.clip_rect.has_value() &&
                 batch.key.primitive_kind ==
                     cgpui::RendererPrimitiveKind::text &&
                 batch.command_count == 1 &&
                 batch.command_indices.size() == 1 &&
                 report.command_count() == 2 &&
                 report.unsupported_commands[0].message == "text_caret" &&
                 accessibility_update.root_element_id == 2 &&
                 accessibility_update.node_count == 3 &&
                 accessibility_update.focused_node_count == 1 &&
                 accessibility_update.nodes[0].role ==
                     cgpui::PlatformAccessibilityRole::text_input
             ? 0
             : 1;
}
