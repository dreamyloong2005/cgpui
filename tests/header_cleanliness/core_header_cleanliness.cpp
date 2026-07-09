#include "cgpui/core/error.hpp"
#include "cgpui/core/event_drag_drop.hpp"
#include "cgpui/core/event_keyboard.hpp"
#include "cgpui/core/event_platform.hpp"
#include "cgpui/core/event_pointer.hpp"
#include "cgpui/core/event_text.hpp"
#include "cgpui/core/event_window.hpp"
#include "cgpui/core/events.hpp"
#include "cgpui/core/entity.hpp"
#include "cgpui/core/geometry.hpp"
#include "cgpui/core/window.hpp"
#include "cgpui/platform/clipboard.hpp"
#include "cgpui/platform/native_surface.hpp"
#include "cgpui/platform/platform_accessibility.hpp"
#include "cgpui/platform/platform_application.hpp"
#include "cgpui/platform/platform_diagnostics.hpp"
#include "cgpui/platform/platform_file_dialog.hpp"
#include "cgpui/platform/platform_native_menu.hpp"
#include "cgpui/platform/platform_window.hpp"
#include "cgpui/platform/platform_window_chrome.hpp"
#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/glyph_atlas_types.hpp"
#include "cgpui/renderer/glyph_cache.hpp"
#include "cgpui/renderer/glyph_texture_resources.hpp"
#include "cgpui/renderer/glyph_uploads.hpp"
#include "cgpui/renderer/image_uploads.hpp"
#include "cgpui/renderer/renderer_frame_reports.hpp"
#include "cgpui/renderer/renderer_geometry_reports.hpp"
#include "cgpui/renderer/renderer_image_reports.hpp"
#include "cgpui/renderer/renderer_submission_reports.hpp"
#include "cgpui/renderer/renderer_text_reports.hpp"
#include "cgpui/renderer/renderer.hpp"

int main() {
  cgpui::WindowDescriptor descriptor;
  descriptor.chrome = cgpui::WindowChromeOptions{
      .titlebar_visible = false,
      .decorations = false,
      .resizable = false,
      .transparent_background = true,
  };
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
  cgpui::ImeComposition styled_ime{
      .phase = cgpui::ImeCompositionPhase::update,
      .text = "styled"};
  (void)cgpui::append_ime_preedit_style(
      styled_ime,
      cgpui::ImePreeditStyleSpan{
          .byte_offset = 0,
          .byte_length = 6,
          .kind = cgpui::ImePreeditStyleKind::underline,
      });
  (void)cgpui::append_ime_default_preedit_style(styled_ime);
  (void)ime_event;
  cgpui::PlatformEvent ime_delete_event = cgpui::ImeDeleteSurroundingText{
      .before_length = 1,
      .after_length = 2,
  };
  (void)ime_delete_event;
  cgpui::DragDropPayload drag_payload{
      .kind = cgpui::DragDropPayloadKind::files,
      .text = {},
      .files = {"C:\\Temp\\first.txt"},
  };
  const cgpui::DragDropAction drag_action = cgpui::DragDropAction::copy;
  cgpui::PlatformEvent drag_event = cgpui::DragEntered{
      .position = {1.0F, 2.0F},
      .payload = drag_payload,
      .action = drag_action};
  (void)drag_event;
  const cgpui::PlatformDiagnosticEvent platform_diagnostic{
      .kind = cgpui::PlatformDiagnosticKind::drag_drop,
      .event_kind = cgpui::EventKind::drag_entered,
      .backend = "header",
      .operation = "drag-entered",
      .supported = true,
      .succeeded = true,
      .value_count = 1,
      .sequence = 1,
  };
  (void)platform_diagnostic;

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
  const cgpui::GlyphAtlasTextureResourceRecord texture_resource{
      .page_index = atlas_image.page_index,
      .image = atlas_image,
      .status = cgpui::GlyphAtlasTextureResourceStatus::created,
      .generation = 1,
  };
  const cgpui::GlyphAtlasTextureResourcePlan texture_plan{
      .live_resources = {texture_resource},
      .created_count = 1,
  };
  const cgpui::GlyphAtlasDirtyUploadRange dirty_upload_range{
      .page_index = atlas_image.page_index,
      .first_upload_index = 0,
      .upload_count = texture_plan.created_count,
      .byte_size = upload_region.byte_size,
  };
  const cgpui::TextSamplerPipelineDescriptor text_sampler_pipeline{
      .primitive_kind = cgpui::RendererPrimitiveKind::text,
      .sampled_image_format = cgpui::GlyphAtlasImageFormat::alpha8_unorm,
      .uses_alpha_sampling = true,
      .uses_text_color = true,
  };
  cgpui::GlyphAtlasTextureResourceState texture_state;
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
  const cgpui::TextSelectionDraw text_selection{
      .rect = cgpui::Rect{.size = {18.0F, 18.0F}},
      .color = cgpui::Color{.r = 0.2F, .g = 0.4F, .b = 0.8F, .a = 0.5F},
      .range = cgpui::TextSelectionRange{.start = 0, .end = 1, .collapsed = false},
      .font_size = 18.0F,
      .clip_rect = cgpui::Rect{.size = {20.0F, 20.0F}},
      .metadata = paint_metadata,
  };
  const cgpui::TextCaretDraw text_caret{
      .rect = cgpui::Rect{.size = {1.0F, 18.0F}},
      .color = cgpui::Color{.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      .byte_offset = 1,
      .font_size = 18.0F,
      .clip_rect = cgpui::Rect{.size = {20.0F, 20.0F}},
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
  const cgpui::RoundedRectDraw rounded_rect{
      .rect = cgpui::Rect{.size = {18.0F, 12.0F}},
      .color = cgpui::Color{.r = 0.25F, .g = 0.5F, .b = 0.75F, .a = 1.0F},
      .radius = cgpui::BorderRadii::corners(1.0F, 2.0F, 3.0F, 4.0F),
      .clip_rect = cgpui::Rect{.size = {20.0F, 14.0F}},
      .metadata = paint_metadata,
  };
  const cgpui::RoundedRectTessellationRecord rounded_tessellation{
      .rect = rounded_rect.rect,
      .color = rounded_rect.color,
      .radius = rounded_rect.radius,
      .clip_rect = rounded_rect.clip_rect,
      .metadata = rounded_rect.metadata,
      .corner_segment_count = 4,
      .vertex_count = 20,
      .triangle_count = 18,
  };
  const cgpui::TextSelectionGeometryRecord text_selection_geometry{
      .rect = text_selection.rect,
      .color = text_selection.color,
      .range = text_selection.range,
      .font_size = text_selection.font_size,
      .clip_rect = text_selection.clip_rect,
      .metadata = text_selection.metadata,
      .vertex_count = 4,
      .triangle_count = 2,
  };
  const cgpui::TextCaretGeometryRecord text_caret_geometry{
      .rect = text_caret.rect,
      .color = text_caret.color,
      .byte_offset = text_caret.byte_offset,
      .font_size = text_caret.font_size,
      .clip_rect = text_caret.clip_rect,
      .metadata = text_caret.metadata,
      .vertex_count = 4,
      .triangle_count = 2,
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
  const auto unknown_primitive =
      static_cast<cgpui::RendererPrimitiveKind>(999);
  const cgpui::RendererCommandStreamItem unsupported_command{
      .primitive_kind = unknown_primitive,
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
      .rounded_rect_tessellations = {rounded_tessellation},
      .text_selection_geometries = {text_selection_geometry},
      .text_caret_geometries = {text_caret_geometry},
      .supported_command_count = 1,
      .unsupported_command_count = 1,
      .rounded_rect_tessellation_count = 1,
      .text_selection_geometry_count = 1,
      .text_caret_geometry_count = 1,
      .text_render =
          cgpui::RendererTextRenderReport{
              .text_sampler_pipeline = text_sampler_pipeline,
              .text_sampler_pipeline_descriptor_count = 1,
              .text_sampler_pipeline_pending_text_draw_count = 1,
          },
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
  const cgpui::NativeMenuModel native_menu{
      .items =
          {
              cgpui::NativeMenuItem{
                  .kind = cgpui::NativeMenuItemKind::command,
                  .title = "Save",
                  .action_name = "file.save",
                  .accelerator =
                      cgpui::NativeMenuAccelerator{
                          .key_code = 'S',
                          .modifiers =
                              cgpui::KeyboardModifiers{.control = true},
                      },
              },
          },
  };
  const cgpui::PlatformMenuInstallationResult native_menu_install_result{
      .supported = true,
      .backend = "test",
      .menu_count = native_menu.items.size(),
      .item_count = cgpui::native_menu_item_count(native_menu),
      .accelerator_count =
          cgpui::native_menu_accelerator_count(native_menu),
  };
  const cgpui::NativeFileDialogOptions native_file_dialog_options{
      .kind = cgpui::NativeFileDialogKind::save_file,
      .title = "Save Project",
      .default_directory = "D:/Projects",
      .suggested_name = "project.cgpui",
      .filters =
          {
              cgpui::NativeFileDialogFilter{
                  .name = "CGPUI Project",
                  .extensions = {"cgpui"},
              },
          },
  };
  const cgpui::NativeFileDialogResult native_file_dialog_result{
      .supported = false,
      .accepted = false,
      .backend = "test",
      .kind = native_file_dialog_options.kind,
      .filter_count = native_file_dialog_options.filters.size(),
  };
  const cgpui::PlatformWindowChromeState platform_window_chrome{
      .supported = false,
      .backend = "test",
      .requested = descriptor.chrome,
      .applied = cgpui::WindowChromeOptions{},
      .reason = "unsupported",
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
                 texture_resource.generation == 1 &&
                 texture_plan.created_count == 1 &&
                 dirty_upload_range.byte_size == upload_region.byte_size &&
                 !text_sampler_pipeline.ready() &&
                 text_sampler_pipeline.sampled_image_format ==
                     cgpui::GlyphAtlasImageFormat::alpha8_unorm &&
                 texture_state.live_resources().empty() &&
                 atlas_pages.size() == 1 &&
                 textured_quad.page_index == 0 &&
                 textured_quad.atlas_uv_bounds.size.width > 0.0F &&
                 rounded_rect.radius.bottom_left == 4.0F &&
                 rounded_tessellation.vertex_count == 20 &&
                 text_selection_geometry.range.end == 1 &&
                 text_caret_geometry.byte_offset == 1 &&
                 text_draw.glyphs.size() == 1 &&
                 text_draw.metadata.transform.translate_x == 4.0F &&
                 text_draw.clip_rect.has_value() &&
                 batch.key.primitive_kind ==
                     cgpui::RendererPrimitiveKind::text &&
                 batch.command_count == 1 &&
                 batch.command_indices.size() == 1 &&
                 report.command_count() == 2 &&
                 report.rounded_rect_tessellation_count == 1 &&
                 report.text_selection_geometry_count == 1 &&
                 report.text_caret_geometry_count == 1 &&
                 report.text_render
                         .text_sampler_pipeline_pending_text_draw_count == 1 &&
                 report.unsupported_commands[0].message == "unknown" &&
                 accessibility_update.root_element_id == 2 &&
                 accessibility_update.node_count == 3 &&
                 accessibility_update.focused_node_count == 1 &&
                 accessibility_update.nodes[0].role ==
                     cgpui::PlatformAccessibilityRole::text_input &&
                 native_menu_install_result.accelerator_count == 1 &&
                 native_menu.items[0].action_name == "file.save" &&
                 native_file_dialog_result.kind ==
                     cgpui::NativeFileDialogKind::save_file &&
                 native_file_dialog_result.filter_count == 1 &&
                 platform_window_chrome.requested.transparent_background &&
                 !platform_window_chrome.applied.transparent_background
             ? 0
             : 1;
}
