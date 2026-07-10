#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

std::filesystem::path executable_directory;

std::string read_file(const std::filesystem::path& path) {
  std::ifstream source(path);
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

std::string read_source_from_project_root(
    std::filesystem::path directory,
    const std::filesystem::path& relative_path) {
  if (directory.empty()) {
    return {};
  }
  directory = std::filesystem::absolute(directory);
  for (; !directory.empty(); directory = directory.parent_path()) {
    if (std::filesystem::exists(directory / ".git")) {
      const std::string text = read_file(directory / relative_path);
      if (!text.empty()) {
        return text;
      }
    }
    if (directory == directory.root_path()) {
      break;
    }
  }
  return {};
}

std::string read_source_from_nearest_match(
    std::filesystem::path directory,
    const std::filesystem::path& relative_path) {
  if (directory.empty()) {
    return {};
  }
  directory = std::filesystem::absolute(directory);
  for (; !directory.empty(); directory = directory.parent_path()) {
    const std::string text = read_file(directory / relative_path);
    if (!text.empty()) {
      return text;
    }
    if (directory == directory.root_path()) {
      break;
    }
  }

  return {};
}

std::string read_source(const char* path) {
  const std::filesystem::path relative_path(path);
  if (const char* source_root = std::getenv("CGPUI_SOURCE_ROOT");
      source_root != nullptr) {
    const std::string text =
        read_file(std::filesystem::path(source_root) / relative_path);
    if (!text.empty()) {
      return text;
    }
  }

  std::string text =
      read_source_from_project_root(executable_directory, relative_path);
  if (!text.empty()) {
    return text;
  }

  text = read_source_from_project_root(
      std::filesystem::current_path(),
      relative_path);
  if (!text.empty()) {
    return text;
  }

  return read_source_from_nearest_match(
      std::filesystem::current_path(),
      relative_path);
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t count = 0;
  for (const char value : text) {
    if (value == '\n') {
      count += 1;
    }
  }
  return count;
}

} // namespace

int main(int argc, char** argv) {
  if (argc > 0 && argv[0] != nullptr) {
    executable_directory = std::filesystem::absolute(argv[0]).parent_path();
  }

  const std::vector<const char*> vulkan_files{
      "include/cgpui/renderer/renderer_types.hpp",
      "include/cgpui/renderer/renderer_commands.hpp",
      "include/cgpui/renderer/renderer_reports.hpp",
      "include/cgpui/renderer/renderer_text_reports.hpp",
      "include/cgpui/renderer/renderer_image_reports.hpp",
      "include/cgpui/renderer/renderer_submission_reports.hpp",
      "include/cgpui/renderer/renderer_geometry_reports.hpp",
      "include/cgpui/renderer/renderer_frame_reports.hpp",
      "include/cgpui/renderer/glyph_atlas_types.hpp",
      "include/cgpui/renderer/glyph_uploads.hpp",
      "include/cgpui/renderer/image_uploads.hpp",
      "include/cgpui/renderer/svg_image_upload.hpp",
      "src/renderer/svg_image_upload.cpp",
      "include/cgpui/renderer/svg_raster_cache.hpp",
      "src/renderer/svg_raster_cache.cpp",
      "include/cgpui/renderer/svg_raster_colorization.hpp",
      "src/renderer/svg_raster_colorization_internal.hpp",
      "src/renderer/svg_raster_colorization.cpp",
      "include/cgpui/renderer/svg_rasterization.hpp",
      "src/renderer/svg_rasterization.cpp",
      "src/renderer/svg_rasterization_lunasvg.cpp",
      "include/cgpui/renderer/svg_viewport_scaling.hpp",
      "src/renderer/svg_viewport_scaling.cpp",
      "include/cgpui/renderer/glyph_texture_resources.hpp",
      "include/cgpui/renderer/glyph_atlas_production.hpp",
      "include/cgpui/renderer/glyph_cache.hpp",
      "include/cgpui/renderer/glyph_atlas.hpp",
      "include/cgpui/renderer/renderer_frame.hpp",
      "include/cgpui/renderer/renderer.hpp",
      "include/cgpui/platform/platform_accessibility.hpp",
      "include/cgpui/platform/platform_native_menu.hpp",
      "include/cgpui/platform/platform_file_dialog.hpp",
      "include/cgpui/platform/platform_window_chrome.hpp",
      "include/cgpui/platform/platform_diagnostics.hpp",
      "include/cgpui/platform/platform_window.hpp",
      "include/cgpui/platform/platform_application.hpp",
      "include/cgpui/platform/platform.hpp",
      "include/cgpui/core/event_window.hpp",
      "include/cgpui/core/event_pointer.hpp",
      "include/cgpui/core/event_drag_drop.hpp",
      "include/cgpui/core/event_keyboard.hpp",
      "include/cgpui/core/event_text.hpp",
      "include/cgpui/core/event_platform.hpp",
      "include/cgpui/core/events.hpp",
      "src/core/event_text.cpp",
      "src/renderer/vulkan/vulkan_internal.hpp",
      "src/renderer/vulkan/vulkan_platform_internal.hpp",
      "src/renderer/vulkan/vulkan_state_internal.hpp",
      "src/renderer/vulkan/vulkan_swapchain_internal.hpp",
      "src/renderer/vulkan/vulkan_device_internal.hpp",
      "src/renderer/vulkan/vulkan_command_recording_internal.hpp",
      "src/renderer/vulkan/vulkan_frame_command_reuse_internal.hpp",
      "src/renderer/vulkan/vulkan_frame_command_reuse.cpp",
      "src/renderer/renderer_frame_diagnostic_snapshot.cpp",
      "src/renderer/vulkan/vulkan_frame_diagnostic_snapshot_internal.hpp",
      "src/renderer/vulkan/vulkan_frame_diagnostic_resources.cpp",
      "src/renderer/vulkan/vulkan_frame_diagnostic_snapshot.cpp",
      "src/renderer/vulkan/vulkan_frame_diagnostic_timing.cpp",
      "src/renderer/vulkan/vulkan_frame_submission.cpp",
      "tests/renderer/vulkan_frame_diagnostic_snapshot_test.cpp",
      "src/renderer/vulkan/vulkan_frame_draw_order_internal.hpp",
      "src/renderer/vulkan/vulkan_frame_draw_order.cpp",
      "src/renderer/vulkan/vulkan_frame_draw_recording_internal.hpp",
      "src/renderer/vulkan/vulkan_frame_draw_recording.cpp",
      "src/renderer/vulkan/vulkan_frame_pipeline_switch_internal.hpp",
      "src/renderer/vulkan/vulkan_frame_pipeline_switch.cpp",
      "tests/renderer/vulkan_pipeline_switch_batching_test.cpp",
      "src/renderer/vulkan/vulkan_frame_geometry_buffer_internal.hpp",
      "src/renderer/vulkan/vulkan_frame_geometry_buffer.cpp",
      "tests/renderer/vulkan_frame_command_reuse_test.cpp",
      "src/renderer/vulkan/vulkan_clip_scissor_internal.hpp",
      "src/renderer/vulkan/vulkan_clip_scissor.cpp",
      "src/renderer/vulkan/vulkan_composition_opacity_internal.hpp",
      "src/renderer/vulkan/vulkan_composition_opacity.cpp",
      "src/renderer/vulkan/vulkan_composition_transform_internal.hpp",
      "src/renderer/vulkan/vulkan_composition_transform.cpp",
      "src/renderer/vulkan/vulkan_image_texture_resources_internal.hpp",
      "src/renderer/vulkan/vulkan_image_texture_images.cpp",
      "src/renderer/vulkan/vulkan_image_texture_resources.cpp",
      "src/renderer/vulkan/vulkan_image_texture_frame.cpp",
      "src/renderer/vulkan/vulkan_image_texture_cache_internal.hpp",
      "src/renderer/vulkan/vulkan_image_texture_cache.cpp",
      "src/renderer/vulkan/vulkan_image_texture_invalidation_internal.hpp",
      "src/renderer/vulkan/vulkan_image_texture_invalidation.cpp",
      "src/renderer/vulkan/vulkan_image_texture_descriptors_internal.hpp",
      "src/renderer/vulkan/vulkan_image_texture_samplers.cpp",
      "src/renderer/vulkan/vulkan_image_texture_descriptors.cpp",
      "src/renderer/vulkan/vulkan_image_texture_resource_binding.cpp",
      "src/renderer/vulkan/vulkan_image_color_internal.hpp",
      "src/renderer/vulkan/vulkan_image_color.cpp",
      "src/renderer/vulkan/shaders/image.vert.glsl",
      "src/renderer/vulkan/shaders/image.frag.glsl",
      "src/renderer/vulkan/vulkan_image_pipeline_internal.hpp",
      "src/renderer/vulkan/vulkan_image_pipeline_resources_internal.hpp",
      "src/renderer/vulkan/vulkan_image_pipeline_state.cpp",
      "src/renderer/vulkan/vulkan_image_pipeline_resources.cpp",
      "src/renderer/vulkan/vulkan_image_shader_binaries.cpp",
      "src/renderer/vulkan/vulkan_image_shader_modules.cpp",
      "src/renderer/vulkan/vulkan_image_vertex_buffer_internal.hpp",
      "src/renderer/vulkan/vulkan_image_vertex_buffer.cpp",
      "src/renderer/vulkan/vulkan_image_draw_recording_internal.hpp",
      "src/renderer/vulkan/vulkan_image_draw_recording.cpp",
      "src/renderer/vulkan/vulkan_image_texture_uploads_internal.hpp",
      "src/renderer/vulkan/vulkan_image_texture_staging.cpp",
      "src/renderer/vulkan/vulkan_image_texture_upload_recording.cpp",
      "src/renderer/vulkan/vulkan_upload_barrier_batch_internal.hpp",
      "src/renderer/vulkan/vulkan_upload_barrier_batch.cpp",
      "tests/renderer/vulkan_upload_barrier_batching_test.cpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_uploads_internal.hpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_resources_internal.hpp",
      "src/renderer/vulkan/vulkan_command_recording.cpp",
      "src/renderer/vulkan/vulkan_solid_rect_geometry_internal.hpp",
      "src/renderer/vulkan/vulkan_solid_rect_geometry.cpp",
      "src/renderer/vulkan/vulkan_solid_rect_frame.cpp",
      "src/renderer/vulkan/vulkan_device.cpp",
      "src/renderer/vulkan/vulkan_device_memory.cpp",
      "src/renderer/vulkan/vulkan_errors.cpp",
      "src/renderer/vulkan/vulkan_helpers.cpp",
      "src/renderer/vulkan/vulkan_instance_surface.cpp",
      "src/renderer/vulkan/vulkan_physical_device.cpp",
      "src/renderer/vulkan/vulkan_renderer.cpp",
      "src/renderer/vulkan/vulkan_report.cpp",
      "src/renderer/vulkan/vulkan_report_commands.cpp",
      "src/renderer/vulkan/vulkan_report_frame.cpp",
      "src/renderer/vulkan/vulkan_report_geometry.cpp",
      "src/renderer/vulkan/vulkan_report_glyph_uploads.cpp",
      "src/renderer/vulkan/vulkan_report_image_uploads.cpp",
      "src/renderer/vulkan/vulkan_report_internal.hpp",
      "src/renderer/vulkan/vulkan_report_keys.cpp",
      "src/renderer/vulkan/vulkan_report_payloads.cpp",
      "src/renderer/vulkan/vulkan_report_submission.cpp",
      "src/renderer/vulkan/vulkan_report_text_submission.cpp",
      "src/renderer/vulkan/vulkan_report_submission_stats.cpp",
      "src/renderer/vulkan/vulkan_report_stream.cpp",
      "src/renderer/vulkan/vulkan_report_text_draws.cpp",
      "src/renderer/vulkan/vulkan_report_text_quads.cpp",
      "src/renderer/vulkan/vulkan_report_texture_resources.cpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_production.cpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_draw_bindings_internal.hpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_draw_bindings.cpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_draw_data.cpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_descriptors.cpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_frame.cpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_images.cpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_resources.cpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_staging.cpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_upload_recording.cpp",
      "src/renderer/vulkan/vulkan_report_uploads.cpp",
      "src/renderer/vulkan/vulkan_presentation.cpp",
      "src/renderer/vulkan/vulkan_presentation_recovery.cpp",
      "src/renderer/vulkan/vulkan_swapchain_recovery_policy_internal.hpp",
      "src/renderer/vulkan/vulkan_swapchain_recovery_policy.cpp",
      "src/renderer/vulkan/vulkan_swapchain_recovery.cpp",
      "tests/renderer/vulkan_swapchain_recovery_test.cpp",
      "src/renderer/vulkan/vulkan_resize.cpp",
      "src/renderer/vulkan/vulkan_state.cpp",
      "src/renderer/vulkan/vulkan_surface_selection.cpp",
      "src/renderer/vulkan/vulkan_sync.cpp",
      "src/renderer/vulkan/vulkan_text_pipeline_internal.hpp",
      "src/renderer/vulkan/vulkan_text_pipeline_resources_internal.hpp",
      "src/renderer/vulkan/vulkan_text_pipeline_resources.cpp",
      "src/renderer/vulkan/vulkan_text_pipeline_state.cpp",
      "src/renderer/vulkan/vulkan_text_draw_recording_internal.hpp",
      "src/renderer/vulkan/vulkan_text_draw_recording.cpp",
      "src/renderer/vulkan/vulkan_text_coverage_internal.hpp",
      "src/renderer/vulkan/vulkan_text_coverage.cpp",
      "src/renderer/vulkan/vulkan_text_positioning_internal.hpp",
      "src/renderer/vulkan/vulkan_text_positioning.cpp",
      "src/renderer/vulkan/vulkan_rounded_rect_geometry_internal.hpp",
      "src/renderer/vulkan/vulkan_rounded_rect_geometry.cpp",
      "src/renderer/vulkan/vulkan_rounded_rect_antialiasing_internal.hpp",
      "src/renderer/vulkan/vulkan_rounded_rect_antialiasing.cpp",
      "src/renderer/vulkan/vulkan_rounded_rect_radii_internal.hpp",
      "src/renderer/vulkan/vulkan_rounded_rect_radii.cpp",
      "src/renderer/vulkan/vulkan_rounded_rect_buffers_internal.hpp",
      "src/renderer/vulkan/vulkan_rounded_rect_buffers.cpp",
      "src/renderer/vulkan/vulkan_rounded_rect_frame.cpp",
      "src/renderer/vulkan/vulkan_rounded_rect_pipeline_internal.hpp",
      "src/renderer/vulkan/vulkan_rounded_rect_pipeline_state.cpp",
      "src/renderer/vulkan/vulkan_rounded_rect_shader_binaries.cpp",
      "src/renderer/vulkan/vulkan_rounded_rect_shader_modules.cpp",
      "src/renderer/vulkan/vulkan_rounded_rect_pipeline_resources.cpp",
      "src/renderer/vulkan/vulkan_rounded_rect_draw_recording_internal.hpp",
      "src/renderer/vulkan/vulkan_rounded_rect_draw_recording.cpp",
      "src/renderer/vulkan/vulkan_text_shader_binaries.cpp",
      "src/renderer/vulkan/vulkan_text_shader_modules.cpp",
      "src/renderer/vulkan/vulkan_text_vertex_buffer_internal.hpp",
      "src/renderer/vulkan/vulkan_text_vertex_buffer.cpp",
      "src/renderer/vulkan/vulkan_swapchain.cpp",
      "src/renderer/vulkan/vulkan_swapchain_create.cpp",
      "src/renderer/vulkan/vulkan_swapchain_query.cpp",
      "src/renderer/vulkan/vulkan_swapchain_create_info.cpp",
      "src/renderer/vulkan/vulkan_swapchain_commands.cpp",
      "src/renderer/vulkan/vulkan_swapchain_images.cpp",
      "src/renderer/vulkan/vulkan_swapchain_lifecycle.cpp",
      "src/renderer/vulkan/vulkan_swapchain_render_pass.cpp",
  };
  for (std::size_t index = 0; index < vulkan_files.size(); ++index) {
    if (read_source(vulkan_files[index]).empty()) {
      return 30 + static_cast<int>(index);
    }
  }

  const std::string renderer_header =
      read_source("include/cgpui/renderer/renderer.hpp");
  if (!contains(renderer_header,
                "#include \"cgpui/renderer/renderer_types.hpp\"") ||
      !contains(renderer_header,
                "#include \"cgpui/renderer/renderer_commands.hpp\"") ||
      !contains(renderer_header,
                "#include \"cgpui/renderer/renderer_reports.hpp\"") ||
      !contains(renderer_header,
                "#include \"cgpui/renderer/svg_image_upload.hpp\"") ||
      !contains(renderer_header,
                "#include \"cgpui/renderer/svg_rasterization.hpp\"") ||
      !contains(renderer_header,
                "#include \"cgpui/renderer/svg_raster_cache.hpp\"") ||
      !contains(renderer_header,
                "#include \"cgpui/renderer/svg_raster_colorization.hpp\"") ||
      !contains(renderer_header,
                "#include \"cgpui/renderer/svg_viewport_scaling.hpp\"") ||
      !contains(renderer_header, "#include \"cgpui/renderer/glyph_atlas.hpp\"") ||
      !contains(renderer_header,
                "#include \"cgpui/renderer/renderer_frame.hpp\"")) {
    return 7;
  }
  if (line_count(renderer_header) > 40 ||
      contains(renderer_header, "class Renderer") ||
      contains(renderer_header, "struct RendererCommandReport") ||
      contains(renderer_header, "struct SvgImageUploadResult") ||
      contains(renderer_header, "struct SvgRasterizationRequest") ||
      contains(renderer_header, "struct SvgRasterColorizationPlan") ||
      contains(renderer_header, "struct SvgViewportScalingPlan") ||
      contains(renderer_header, "class SvgRasterCache") ||
      contains(renderer_header, "class GlyphCache")) {
    return 8;
  }

  const std::string renderer_types =
      read_source("include/cgpui/renderer/renderer_types.hpp");
  const std::string renderer_commands =
      read_source("include/cgpui/renderer/renderer_commands.hpp");
  const std::string renderer_reports =
      read_source("include/cgpui/renderer/renderer_reports.hpp");
  const std::string renderer_text_reports =
      read_source("include/cgpui/renderer/renderer_text_reports.hpp");
  const std::string renderer_image_reports =
      read_source("include/cgpui/renderer/renderer_image_reports.hpp");
  const std::string renderer_submission_reports =
      read_source("include/cgpui/renderer/renderer_submission_reports.hpp");
  const std::string renderer_geometry_reports =
      read_source("include/cgpui/renderer/renderer_geometry_reports.hpp");
  const std::string renderer_frame_reports =
      read_source("include/cgpui/renderer/renderer_frame_reports.hpp");
  const std::string renderer_frame_diagnostics =
      read_source("include/cgpui/renderer/renderer_frame_diagnostics.hpp");
  const std::string renderer_frame_diagnostics_source =
      read_source("src/renderer/renderer_frame_diagnostics.cpp");
  const std::string renderer_frame_upload_diagnostics_source =
      read_source("src/renderer/renderer_frame_upload_diagnostics.cpp");
  const std::string renderer_frame_draw_diagnostics_source =
      read_source("src/renderer/renderer_frame_draw_diagnostics.cpp");
  const std::string renderer_dropped_resource_diagnostics = read_source(
      "include/cgpui/renderer/renderer_dropped_resource_diagnostics.hpp");
  const std::string renderer_dropped_resource_diagnostics_source = read_source(
      "src/renderer/renderer_dropped_resource_diagnostics.cpp");
  const std::string renderer_frame_timing_diagnostics = read_source(
      "include/cgpui/renderer/renderer_frame_timing_diagnostics.hpp");
  const std::string renderer_frame_timing_diagnostics_source = read_source(
      "src/renderer/renderer_frame_timing_diagnostics.cpp");
  const std::string glyph_atlas =
      read_source("include/cgpui/renderer/glyph_atlas.hpp");
  const std::string glyph_atlas_types =
      read_source("include/cgpui/renderer/glyph_atlas_types.hpp");
  const std::string glyph_uploads =
      read_source("include/cgpui/renderer/glyph_uploads.hpp");
  const std::string image_uploads =
      read_source("include/cgpui/renderer/image_uploads.hpp");
  const std::string svg_image_upload =
      read_source("include/cgpui/renderer/svg_image_upload.hpp");
  const std::string svg_image_upload_source =
      read_source("src/renderer/svg_image_upload.cpp");
  const std::string svg_raster_cache =
      read_source("include/cgpui/renderer/svg_raster_cache.hpp");
  const std::string svg_raster_cache_source =
      read_source("src/renderer/svg_raster_cache.cpp");
  const std::string svg_raster_colorization =
      read_source("include/cgpui/renderer/svg_raster_colorization.hpp");
  const std::string svg_raster_colorization_internal =
      read_source("src/renderer/svg_raster_colorization_internal.hpp");
  const std::string svg_raster_colorization_source =
      read_source("src/renderer/svg_raster_colorization.cpp");
  const std::string svg_rasterization =
      read_source("include/cgpui/renderer/svg_rasterization.hpp");
  const std::string svg_rasterization_source =
      read_source("src/renderer/svg_rasterization.cpp");
  const std::string svg_rasterization_lunasvg =
      read_source("src/renderer/svg_rasterization_lunasvg.cpp");
  const std::string svg_viewport_scaling =
      read_source("include/cgpui/renderer/svg_viewport_scaling.hpp");
  const std::string svg_viewport_scaling_source =
      read_source("src/renderer/svg_viewport_scaling.cpp");
  const std::string glyph_texture_resources =
      read_source("include/cgpui/renderer/glyph_texture_resources.hpp");
  const std::string glyph_atlas_production =
      read_source("include/cgpui/renderer/glyph_atlas_production.hpp");
  const std::string glyph_cache =
      read_source("include/cgpui/renderer/glyph_cache.hpp");
  const std::string renderer_frame =
      read_source("include/cgpui/renderer/renderer_frame.hpp");
  if (!contains(renderer_reports,
                "#include \"cgpui/renderer/renderer_text_reports.hpp\"") ||
      !contains(renderer_reports,
                "#include \"cgpui/renderer/renderer_image_reports.hpp\"") ||
      !contains(renderer_reports,
                "#include \"cgpui/renderer/renderer_submission_reports.hpp\"") ||
      !contains(renderer_reports,
                "#include \"cgpui/renderer/renderer_geometry_reports.hpp\"") ||
      !contains(renderer_reports,
                "#include \"cgpui/renderer/renderer_frame_reports.hpp\"") ||
      !contains(renderer_reports,
                "#include \"cgpui/renderer/renderer_frame_diagnostics.hpp\"") ||
      !contains(
          renderer_reports,
          "#include \"cgpui/renderer/renderer_dropped_resource_diagnostics.hpp\"") ||
      !contains(
          renderer_reports,
          "#include \"cgpui/renderer/renderer_frame_timing_diagnostics.hpp\"") ||
      line_count(renderer_reports) > 40 ||
      contains(renderer_reports, "struct RendererCommandReport") ||
      contains(renderer_reports, "struct RendererFrameReport") ||
      contains(renderer_reports, "struct RendererFrameDiagnostics") ||
      contains(renderer_reports, "struct RendererDroppedResourceDiagnostics") ||
      contains(renderer_reports, "struct RendererFrameTimingDiagnostics")) {
    return 51;
  }
  if (!contains(glyph_atlas,
                "#include \"cgpui/renderer/glyph_atlas_types.hpp\"") ||
      !contains(glyph_atlas, "#include \"cgpui/renderer/glyph_uploads.hpp\"") ||
      !contains(glyph_atlas, "#include \"cgpui/renderer/image_uploads.hpp\"") ||
      !contains(glyph_atlas,
                "#include \"cgpui/renderer/glyph_texture_resources.hpp\"") ||
      !contains(glyph_atlas,
                "#include \"cgpui/renderer/glyph_atlas_production.hpp\"") ||
      !contains(glyph_atlas, "#include \"cgpui/renderer/glyph_cache.hpp\"") ||
      line_count(glyph_atlas) > 40 ||
      contains(glyph_atlas, "class GlyphCache") ||
      contains(glyph_atlas, "class GlyphAtlasTextureResourceState")) {
    return 52;
  }
  if (!contains(renderer_types, "struct RenderSurfaceDescriptor") ||
      !contains(renderer_commands, "struct TextDraw") ||
      !contains(renderer_text_reports, "struct RendererTextRenderReport") ||
      !contains(renderer_image_reports, "struct RendererImageRenderReport") ||
      !contains(renderer_submission_reports,
                "struct RendererSubmissionPlanRecord") ||
      !contains(renderer_geometry_reports,
                "struct RoundedRectTessellationRecord") ||
      !contains(renderer_frame_reports, "struct RendererFrameReport") ||
      !contains(renderer_frame_diagnostics, "struct RendererFrameWork") ||
      !contains(renderer_frame_diagnostics,
                "struct RendererFrameDiagnostics") ||
      !contains(renderer_frame_diagnostics,
                "compare_renderer_frame_work(") ||
      !contains(renderer_frame_diagnostics_source,
                "RendererFrameDiagnostics::exact_match()") ||
      !contains(renderer_frame_diagnostics_source,
                "compare_renderer_frame_work(") ||
      !contains(renderer_frame_diagnostics,
                "struct RendererUploadByteCounts") ||
      !contains(renderer_frame_diagnostics,
                "renderer_upload_byte_counts(") ||
      !contains(renderer_frame_upload_diagnostics_source,
                "renderer_upload_byte_counts(") ||
      !contains(renderer_frame_diagnostics, "struct RendererDrawCounts") ||
      !contains(renderer_frame_diagnostics, "renderer_add_draw_count(") ||
      !contains(renderer_frame_draw_diagnostics_source,
                "renderer_add_draw_count(") ||
      !contains(renderer_dropped_resource_diagnostics,
                "struct RendererFrameResource") ||
      !contains(renderer_dropped_resource_diagnostics,
                "struct RendererDroppedResourceDiagnostics") ||
      !contains(renderer_dropped_resource_diagnostics,
                "classify_renderer_dropped_resources(") ||
      !contains(renderer_dropped_resource_diagnostics_source,
                "classify_renderer_dropped_resources(") ||
      !contains(renderer_frame_timing_diagnostics,
                "struct RendererFrameTimings") ||
      !contains(renderer_frame_timing_diagnostics,
                "struct RendererFrameTimingDiagnostics") ||
      !contains(renderer_frame_timing_diagnostics_source,
                "renderer_add_frame_timing(") ||
      !contains(renderer_frame_timing_diagnostics_source,
                "compare_renderer_frame_timing(") ||
      contains(renderer_frame, "struct RendererFrameDiagnostics") ||
      !contains(glyph_atlas_types, "struct GlyphAtlasEntry") ||
      !contains(glyph_uploads, "struct GlyphAtlasUploadBatch") ||
      !contains(image_uploads, "struct ImageUploadBatch") ||
      !contains(svg_image_upload, "struct SvgImageUploadResult") ||
      !contains(svg_image_upload_source, "upload_svg_image(") ||
      !contains(svg_raster_cache, "class SvgRasterCache") ||
      !contains(svg_raster_cache_source, "SvgRasterCache::rasterize(") ||
      !contains(svg_raster_colorization,
                "struct SvgRasterColorizationPlan") ||
      !contains(svg_raster_colorization_internal,
                "svg_current_color_css(") ||
      !contains(svg_raster_colorization_source,
                "plan_svg_raster_colorization(") ||
      !contains(svg_rasterization, "struct SvgRasterizationRequest") ||
      !contains(svg_rasterization, "struct SvgRasterizationPlan") ||
      !contains(svg_rasterization, "struct SvgRasterizationResult") ||
      !contains(svg_rasterization_source, "plan_svg_rasterization(") ||
      !contains(svg_rasterization_lunasvg,
                "lunasvg::Document::loadFromData(") ||
      !contains(svg_rasterization_lunasvg, "convertToRGBA()") ||
      !contains(svg_viewport_scaling, "struct SvgViewportScalingPlan") ||
      !contains(svg_viewport_scaling_source,
                "plan_svg_viewport_scaling(") ||
      !contains(glyph_texture_resources,
                "class GlyphAtlasTextureResourceState") ||
      !contains(glyph_atlas_production,
                "class GlyphAtlasProductionResourceState") ||
      !contains(glyph_cache, "class GlyphCache") ||
      !contains(renderer_frame, "class Renderer")) {
    return 9;
  }
  if (line_count(glyph_atlas_types) > 220 ||
      line_count(glyph_uploads) > 220 ||
      line_count(image_uploads) > 220 ||
      line_count(svg_image_upload) > 80 ||
      line_count(svg_image_upload_source) > 80 ||
      line_count(svg_raster_cache) > 100 ||
      line_count(svg_raster_cache_source) > 140 ||
      line_count(svg_raster_colorization) > 100 ||
      line_count(svg_raster_colorization_internal) > 40 ||
      line_count(svg_raster_colorization_source) > 100 ||
      line_count(svg_rasterization) > 90 ||
      line_count(svg_rasterization_source) > 120 ||
      line_count(svg_rasterization_lunasvg) > 100 ||
      line_count(svg_viewport_scaling) > 100 ||
      line_count(svg_viewport_scaling_source) > 140 ||
      line_count(glyph_texture_resources) > 220 ||
      line_count(glyph_atlas_production) > 220 ||
      line_count(glyph_cache) > 220 ||
      line_count(renderer_text_reports) > 220 ||
      line_count(renderer_image_reports) > 220 ||
      line_count(renderer_submission_reports) > 220 ||
      line_count(renderer_geometry_reports) > 220 ||
      line_count(renderer_frame_reports) > 220 ||
      line_count(renderer_frame_diagnostics) > 100 ||
      line_count(renderer_frame_diagnostics_source) > 100 ||
      line_count(renderer_frame_upload_diagnostics_source) > 100 ||
      line_count(renderer_frame_draw_diagnostics_source) > 100 ||
      line_count(renderer_dropped_resource_diagnostics) > 100 ||
      line_count(renderer_dropped_resource_diagnostics_source) > 100 ||
      line_count(renderer_frame_timing_diagnostics) > 100 ||
      line_count(renderer_frame_timing_diagnostics_source) > 100) {
    return 53;
  }

  const std::string platform_header =
      read_source("include/cgpui/platform/platform.hpp");
  const std::string platform_accessibility_header =
      read_source("include/cgpui/platform/platform_accessibility.hpp");
  const std::string platform_native_menu_header =
      read_source("include/cgpui/platform/platform_native_menu.hpp");
  const std::string platform_file_dialog_header =
      read_source("include/cgpui/platform/platform_file_dialog.hpp");
  const std::string platform_window_chrome_header =
      read_source("include/cgpui/platform/platform_window_chrome.hpp");
  const std::string platform_diagnostics_header =
      read_source("include/cgpui/platform/platform_diagnostics.hpp");
  const std::string platform_window_header =
      read_source("include/cgpui/platform/platform_window.hpp");
  const std::string platform_application_header =
      read_source("include/cgpui/platform/platform_application.hpp");
  if (!contains(platform_header,
                "#include \"cgpui/platform/platform_accessibility.hpp\"") ||
      !contains(platform_header,
                "#include \"cgpui/platform/platform_native_menu.hpp\"") ||
      !contains(platform_header,
                "#include \"cgpui/platform/platform_file_dialog.hpp\"") ||
      !contains(platform_header,
                "#include \"cgpui/platform/platform_window_chrome.hpp\"") ||
      !contains(platform_header,
                "#include \"cgpui/platform/platform_diagnostics.hpp\"") ||
      !contains(platform_header,
                "#include \"cgpui/platform/platform_window.hpp\"") ||
      !contains(platform_header,
                "#include \"cgpui/platform/platform_application.hpp\"") ||
      line_count(platform_header) > 40 ||
      contains(platform_header, "class PlatformWindow") ||
      contains(platform_header, "class PlatformApplication") ||
      contains(platform_header, "struct PlatformAccessibilityTreeUpdate")) {
    return 54;
  }
  if (!contains(platform_accessibility_header,
                "struct PlatformAccessibilityTreeUpdate") ||
      !contains(platform_native_menu_header, "struct NativeMenuModel") ||
      !contains(platform_file_dialog_header,
                "struct NativeFileDialogOptions") ||
      !contains(platform_window_chrome_header,
                "struct PlatformWindowChromeState") ||
      !contains(platform_diagnostics_header,
                "struct PlatformDiagnosticEvent") ||
      !contains(platform_window_header, "class PlatformWindow") ||
      !contains(platform_application_header, "class PlatformApplication")) {
    return 55;
  }

  const std::string events_header =
      read_source("include/cgpui/core/events.hpp");
  const std::string event_window_header =
      read_source("include/cgpui/core/event_window.hpp");
  const std::string event_pointer_header =
      read_source("include/cgpui/core/event_pointer.hpp");
  const std::string event_drag_drop_header =
      read_source("include/cgpui/core/event_drag_drop.hpp");
  const std::string event_keyboard_header =
      read_source("include/cgpui/core/event_keyboard.hpp");
  const std::string event_text_header =
      read_source("include/cgpui/core/event_text.hpp");
  const std::string event_text_source =
      read_source("src/core/event_text.cpp");
  const std::string event_platform_header =
      read_source("include/cgpui/core/event_platform.hpp");
  if (!contains(events_header, "#include \"cgpui/core/event_window.hpp\"") ||
      !contains(events_header, "#include \"cgpui/core/event_pointer.hpp\"") ||
      !contains(events_header, "#include \"cgpui/core/event_drag_drop.hpp\"") ||
      !contains(events_header, "#include \"cgpui/core/event_keyboard.hpp\"") ||
      !contains(events_header, "#include \"cgpui/core/event_text.hpp\"") ||
      !contains(events_header, "#include \"cgpui/core/event_platform.hpp\"") ||
      line_count(events_header) > 40 ||
      contains(events_header, "struct WindowResized") ||
      contains(events_header, "struct PointerMoved") ||
      contains(events_header, "using PlatformEvent")) {
    return 56;
  }
  if (!contains(event_window_header, "struct WindowCloseRequested") ||
      !contains(event_pointer_header, "struct PointerMoved") ||
      !contains(event_drag_drop_header, "struct DragEntered") ||
      !contains(event_keyboard_header, "struct KeyboardKey") ||
      !contains(event_text_header, "struct ImeComposition") ||
      !contains(event_text_header, "struct ImePreeditStyleSpan") ||
      !contains(event_text_header, "kImePreeditStyleSpanCapacity") ||
      !contains(event_text_source, "append_ime_default_preedit_style") ||
      contains(event_text_header, "std::vector<ImePreeditStyleSpan>") ||
      !contains(event_platform_header, "using PlatformEvent")) {
    return 57;
  }

  const std::string renderer =
      read_source("src/renderer/vulkan/vulkan_renderer.cpp");
  if (line_count(renderer) > 240) {
    return 2;
  }
  if (!contains(renderer, "class VulkanFrame") ||
      !contains(renderer, "class VulkanRenderer final") ||
      !contains(renderer, "create_renderer(")) {
    return 3;
  }
  if (contains(renderer, "class VulkanRendererState") ||
      contains(renderer, "create_swapchain_resources(") ||
      contains(renderer, "vkQueuePresentKHR") ||
      contains(renderer, "vkCreateSwapchainKHR") ||
      contains(renderer, "record_vulkan_frame_command_buffer(") ||
      contains(renderer, "RendererCommandReport vulkan_build_renderer_command_report(") ||
      contains(renderer, "std::vector<GlyphAtlasUploadBatch> vulkan_plan_glyph_atlas_uploads(") ||
      contains(renderer, "std::vector<TexturedGlyphQuad> vulkan_build_textured_glyph_quads(") ||
      contains(renderer, "Result<void> require_vk_success(") ||
      contains(renderer, "Result<void> record_vulkan_frame_command_buffer(")) {
    return 4;
  }

  const std::string helpers =
      read_source("src/renderer/vulkan/vulkan_helpers.cpp");
  if (line_count(helpers) > 60 ||
      contains(helpers, "Result<void> require_vk_success(") ||
      contains(helpers, "choose_vulkan_surface_format(") ||
      contains(helpers, "record_vulkan_frame_command_buffer(")) {
    return 6;
  }

  const std::string errors =
      read_source("src/renderer/vulkan/vulkan_errors.cpp");
  if (line_count(errors) > 120 ||
      !contains(errors, "Error vulkan_error(") ||
      !contains(errors, "Result<void> require_vk_success(") ||
      !contains(errors, "Result<Win32SurfaceHandle> require_win32_surface(") ||
      !contains(errors,
                "Result<WaylandSurfaceHandle> require_wayland_surface(") ||
      contains(errors, "record_vulkan_frame_command_buffer(")) {
    return 34;
  }

  const std::string surface_selection =
      read_source("src/renderer/vulkan/vulkan_surface_selection.cpp");
  if (line_count(surface_selection) > 120 ||
      !contains(surface_selection, "choose_vulkan_surface_format(") ||
      contains(surface_selection, "choose_vulkan_present_mode(") ||
      !contains(surface_selection, "choose_vulkan_composite_alpha(") ||
      !contains(surface_selection, "choose_vulkan_extent(") ||
      contains(surface_selection, "require_vk_success(")) {
    return 35;
  }

  const std::string present_pacing_header = read_source(
      "src/renderer/vulkan/vulkan_present_pacing_internal.hpp");
  const std::string present_pacing =
      read_source("src/renderer/vulkan/vulkan_present_pacing.cpp");
  const std::string present_pacing_wait =
      read_source("src/renderer/vulkan/vulkan_present_pacing_wait.cpp");
  const std::string present_pacing_test =
      read_source("tests/renderer/vulkan_present_pacing_test.cpp");
  if (line_count(present_pacing_header) > 50 ||
      line_count(present_pacing) > 70 ||
      line_count(present_pacing_wait) > 40 ||
      line_count(present_pacing_test) > 220 ||
      !contains(present_pacing_header, "struct VulkanPresentPacingPlan") ||
      !contains(present_pacing, "vulkan_plan_present_pacing(") ||
      !contains(present_pacing, "VK_PRESENT_MODE_MAILBOX_KHR") ||
      !contains(present_pacing, "VK_PRESENT_MODE_FIFO_KHR") ||
      !contains(present_pacing_wait,
                "VulkanRendererState::wait_for_present_pacing()") ||
      !contains(present_pacing_wait, "vkWaitForFences") ||
      contains(present_pacing, "vkWaitForFences") ||
      contains(present_pacing, "vkAcquireNextImageKHR") ||
      contains(present_pacing, "vkQueuePresentKHR") ||
      contains(present_pacing_wait, "vkAcquireNextImageKHR") ||
      contains(present_pacing_wait, "vkQueuePresentKHR")) {
    return 84;
  }

  const std::string command_recording =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string frame_command_reuse_header = read_source(
      "src/renderer/vulkan/vulkan_frame_command_reuse_internal.hpp");
  const std::string frame_command_reuse = read_source(
      "src/renderer/vulkan/vulkan_frame_command_reuse.cpp");
  const std::string frame_command_reuse_test = read_source(
      "tests/renderer/vulkan_frame_command_reuse_test.cpp");
  if (line_count(frame_command_reuse_header) > 130 ||
      line_count(frame_command_reuse) > 230 ||
      line_count(frame_command_reuse_test) > 340 ||
      !contains(frame_command_reuse_header,
                "struct VulkanFrameCommandReuseState") ||
      !contains(frame_command_reuse,
                "vulkan_plan_frame_command_reuse(") ||
      !contains(frame_command_reuse,
                "vulkan_commit_frame_command_recording(") ||
      contains(frame_command_reuse, "vkResetCommandBuffer") ||
      contains(frame_command_reuse, "vkQueueSubmit")) {
    return 36;
  }
  if (line_count(command_recording) > 180 ||
      !contains(command_recording, "record_vulkan_frame_command_buffer(") ||
      !contains(command_recording, "vulkan_plan_frame_command_reuse(") ||
      !contains(command_recording, "vulkan_commit_frame_command_recording(") ||
      contains(command_recording,
               "VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT") ||
      !contains(command_recording, "vkCmdBeginRenderPass") ||
      !contains(command_recording, "solid_rect_buffers") ||
      !contains(command_recording, "vulkan_record_frame_draws(") ||
      contains(command_recording, "vulkan_record_rounded_rect_draws(") ||
      contains(command_recording, "vulkan_record_text_draws(") ||
      contains(command_recording, "vulkan_record_solid_rects(") ||
      contains(command_recording, "vkCmdClearAttachments") ||
      contains(command_recording, "choose_vulkan_surface_format(")) {
    return 37;
  }
  const std::string diagnostic_snapshot_header = read_source(
      "include/cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp");
  const std::string diagnostic_default = read_source(
      "src/renderer/renderer_frame_diagnostic_snapshot.cpp");
  const std::string diagnostic_private_header = read_source(
      "src/renderer/vulkan/vulkan_frame_diagnostic_snapshot_internal.hpp");
  const std::string diagnostic_resources = read_source(
      "src/renderer/vulkan/vulkan_frame_diagnostic_resources.cpp");
  const std::string diagnostic_snapshot = read_source(
      "src/renderer/vulkan/vulkan_frame_diagnostic_snapshot.cpp");
  const std::string diagnostic_timing = read_source(
      "src/renderer/vulkan/vulkan_frame_diagnostic_timing.cpp");
  const std::string frame_submission = read_source(
      "src/renderer/vulkan/vulkan_frame_submission.cpp");
  const std::string diagnostic_renderer_frame =
      read_source("include/cgpui/renderer/renderer_frame.hpp");
  const std::string diagnostic_presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  if (line_count(diagnostic_snapshot_header) > 60 ||
      line_count(diagnostic_default) > 40 ||
      line_count(diagnostic_private_header) > 100 ||
      line_count(diagnostic_resources) > 160 ||
      line_count(diagnostic_snapshot) > 100 ||
      line_count(diagnostic_timing) > 80 ||
      line_count(frame_submission) > 80 ||
      !contains(diagnostic_snapshot_header,
                "struct RendererFrameDiagnosticSnapshot") ||
      !contains(diagnostic_default,
                "Renderer::last_frame_diagnostic_snapshot() const") ||
      !contains(diagnostic_default, "return nullptr;") ||
      !contains(diagnostic_private_header,
                "struct VulkanFrameDiagnosticResources") ||
      !contains(diagnostic_resources,
                "vulkan_build_frame_diagnostic_resources(") ||
      !contains(diagnostic_snapshot,
                "vulkan_build_frame_diagnostic_snapshot(") ||
      !contains(diagnostic_timing,
                "VulkanFrameDiagnosticTimer::finish_stage(") ||
      !contains(frame_submission, "VulkanRendererState::submit_frame(") ||
      !contains(
          diagnostic_renderer_frame,
          "last_frame_diagnostic_snapshot()") ||
      contains(
          diagnostic_renderer_frame,
          "last_frame_diagnostic_snapshot() const {") ||
      !contains(diagnostic_presentation,
                "last_frame_diagnostic_snapshot_")) {
    return 85;
  }
  const std::string solid_rect_geometry_header = read_source(
      "src/renderer/vulkan/vulkan_solid_rect_geometry_internal.hpp");
  const std::string solid_rect_geometry = read_source(
      "src/renderer/vulkan/vulkan_solid_rect_geometry.cpp");
  const std::string solid_rect_frame =
      read_source("src/renderer/vulkan/vulkan_solid_rect_frame.cpp");
  if (line_count(solid_rect_geometry_header) > 30 ||
      line_count(solid_rect_geometry) > 110 ||
      line_count(solid_rect_frame) > 30 ||
      !contains(solid_rect_geometry, "vulkan_build_solid_rect_geometry(") ||
      !contains(solid_rect_geometry, "vulkan_resolve_effective_clip_rect(") ||
      !contains(solid_rect_frame, "prepare_solid_rect_frame(")) {
    return 72;
  }
  const std::string clip_scissor_header = read_source(
      "src/renderer/vulkan/vulkan_clip_scissor_internal.hpp");
  const std::string clip_scissor =
      read_source("src/renderer/vulkan/vulkan_clip_scissor.cpp");
  if (line_count(clip_scissor_header) > 40 ||
      line_count(clip_scissor) > 130 ||
      !contains(clip_scissor_header, "struct VulkanClipScissorResolution") ||
      !contains(clip_scissor,
                "vulkan_resolve_effective_clip_rect(") ||
      !contains(clip_scissor,
                "vulkan_resolve_clip_stack_scissor(") ||
      !contains(solid_rect_geometry,
                "vulkan_resolve_effective_clip_rect(")) {
    return 73;
  }
  const std::string composition_opacity_header = read_source(
      "src/renderer/vulkan/vulkan_composition_opacity_internal.hpp");
  const std::string composition_opacity = read_source(
      "src/renderer/vulkan/vulkan_composition_opacity.cpp");
  if (line_count(composition_opacity_header) > 30 ||
      line_count(composition_opacity) > 50 ||
      !contains(composition_opacity_header,
                "vulkan_resolve_composed_opacity(") ||
      !contains(composition_opacity_header,
                "vulkan_apply_composed_opacity(") ||
      !contains(solid_rect_geometry,
                "vulkan_apply_composed_opacity(") ||
      contains(command_recording, "vulkan_apply_composed_opacity(")) {
    return 74;
  }
  const std::string composition_transform_header = read_source(
      "src/renderer/vulkan/vulkan_composition_transform_internal.hpp");
  const std::string composition_transform = read_source(
      "src/renderer/vulkan/vulkan_composition_transform.cpp");
  if (line_count(composition_transform_header) > 30 ||
      line_count(composition_transform) > 70 ||
      !contains(composition_transform_header,
                "vulkan_resolve_composed_transform(") ||
      !contains(composition_transform_header, "vulkan_transform_point(") ||
      !contains(composition_transform_header,
                "vulkan_apply_composed_transform(") ||
      !contains(solid_rect_geometry, "vulkan_apply_composed_transform(") ||
      contains(command_recording, "vulkan_apply_composed_transform(")) {
    return 75;
  }
  const std::string frame_draw_order_header = read_source(
      "src/renderer/vulkan/vulkan_frame_draw_order_internal.hpp");
  const std::string frame_draw_order =
      read_source("src/renderer/vulkan/vulkan_frame_draw_order.cpp");
  const std::string frame_draw_recording_header = read_source(
      "src/renderer/vulkan/vulkan_frame_draw_recording_internal.hpp");
  const std::string frame_draw_recording =
      read_source("src/renderer/vulkan/vulkan_frame_draw_recording.cpp");
  const std::string frame_pipeline_switch_header = read_source(
      "src/renderer/vulkan/vulkan_frame_pipeline_switch_internal.hpp");
  const std::string frame_pipeline_switch = read_source(
      "src/renderer/vulkan/vulkan_frame_pipeline_switch.cpp");
  const std::string pipeline_switch_test = read_source(
      "tests/renderer/vulkan_pipeline_switch_batching_test.cpp");
  const std::string draw_order_presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  if (frame_pipeline_switch_header.empty() || frame_pipeline_switch.empty() ||
      pipeline_switch_test.empty() ||
      line_count(frame_pipeline_switch_header) > 60 ||
      line_count(frame_pipeline_switch) > 70 ||
      line_count(pipeline_switch_test) > 230 ||
      !contains(frame_pipeline_switch_header,
                "struct VulkanFramePipelineSwitchState") ||
      !contains(frame_pipeline_switch,
                "vulkan_plan_frame_pipeline_switch(") ||
      contains(frame_pipeline_switch, "vkCmdBindPipeline") ||
      contains(frame_pipeline_switch, "VkCommandBuffer")) {
    return 54;
  }
  if (line_count(frame_draw_order_header) > 80 ||
      line_count(frame_draw_order) > 100 ||
      line_count(frame_draw_recording_header) > 50 ||
      line_count(frame_draw_recording) > 100 ||
      !contains(frame_draw_order_header, "struct VulkanFrameDrawOrderEntry") ||
      !contains(frame_draw_order_header, "class VulkanFrameDrawOrderCursor") ||
      contains(frame_draw_order_header, "std::vector") ||
      contains(frame_draw_order, "std::vector") ||
      !contains(renderer, "draw_order_") ||
      !contains(renderer, "append_draw(") ||
      !contains(draw_order_presentation, "VulkanFrameDrawOrderEntry") ||
      !contains(frame_draw_recording, "VulkanFrameDrawOrderCursor") ||
      !contains(frame_draw_recording,
                "VulkanFramePipelineSwitchState") ||
      !contains(frame_draw_recording, "switch_plan.bind_pipeline") ||
      !contains(frame_draw_recording,
                "vulkan_bind_rounded_rect_draw_state(") ||
      !contains(frame_draw_recording, "vulkan_record_rounded_rect_draw(") ||
      !contains(frame_draw_recording, "vulkan_bind_text_draw_state(") ||
      !contains(frame_draw_recording, "vulkan_record_text_draw(")) {
    return 76;
  }

  const std::string internal =
      read_source("src/renderer/vulkan/vulkan_internal.hpp");
  const std::string platform_internal =
      read_source("src/renderer/vulkan/vulkan_platform_internal.hpp");
  const std::string state_internal =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string swapchain_internal =
      read_source("src/renderer/vulkan/vulkan_swapchain_internal.hpp");
  const std::string device_internal =
      read_source("src/renderer/vulkan/vulkan_device_internal.hpp");
  if (!contains(internal, "#include \"vulkan_platform_internal.hpp\"") ||
      !contains(internal, "#include \"vulkan_state_internal.hpp\"") ||
      !contains(internal, "#include \"vulkan_swapchain_internal.hpp\"") ||
      !contains(internal, "#include \"vulkan_device_internal.hpp\"") ||
      !contains(platform_internal, "require_vk_success(") ||
      !contains(platform_internal, "require_win32_surface(") ||
      !contains(platform_internal, "require_wayland_surface(") ||
      !contains(state_internal, "class VulkanRendererState final") ||
      !contains(state_internal, "present_frame(") ||
      !contains(swapchain_internal, "struct VulkanSwapchainResources") ||
      !contains(device_internal, "struct VulkanQueueFamilySearch") ||
      !contains(device_internal, "struct VulkanQueueFamilies")) {
    return 10;
  }
  if (line_count(internal) > 120 ||
      contains(internal, "class VulkanRendererState final") ||
      contains(internal, "struct VulkanSwapchainResources") ||
      contains(internal, "struct VulkanQueueFamilySearch") ||
      contains(internal, "present_frame(")) {
    return 50;
  }

  const std::string state =
      read_source("src/renderer/vulkan/vulkan_state.cpp");
  if (line_count(state) > 170) {
    return 11;
  }
  if (!contains(state, "VulkanRendererState::VulkanRendererState(") ||
      !contains(state, "VulkanRendererState::~VulkanRendererState()") ||
      !contains(state, "VulkanRendererState::create(") ||
      contains(state, "VulkanRendererState::resize(") ||
      contains(state, "VulkanRendererState::present_frame(") ||
      contains(state, "vkQueuePresentKHR") ||
      contains(state, "record_vulkan_frame_command_buffer(") ||
      contains(state, "vkCreateSemaphore") ||
      contains(state, "vkCreateFence") ||
      contains(state, "vkCreateInstance") ||
      contains(state, "vkCreateSwapchainKHR")) {
    return 12;
  }

  const std::string resize =
      read_source("src/renderer/vulkan/vulkan_resize.cpp");
  if (line_count(resize) > 120 ||
      !contains(resize, "VulkanRendererState::resize(") ||
      !contains(resize, "create_swapchain_resources(") ||
      !contains(resize, "install_swapchain(") ||
      contains(resize, "vkQueuePresentKHR") ||
      contains(resize, "VulkanRendererState::present_frame(")) {
    return 31;
  }

  const std::string presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  const std::string presentation_recovery =
      read_source("src/renderer/vulkan/vulkan_presentation_recovery.cpp");
  const std::string swapchain_recovery_policy_header = read_source(
      "src/renderer/vulkan/vulkan_swapchain_recovery_policy_internal.hpp");
  const std::string swapchain_recovery_policy = read_source(
      "src/renderer/vulkan/vulkan_swapchain_recovery_policy.cpp");
  const std::string swapchain_recovery = read_source(
      "src/renderer/vulkan/vulkan_swapchain_recovery.cpp");
  const std::string swapchain_recovery_test = read_source(
      "tests/renderer/vulkan_swapchain_recovery_test.cpp");
  if (presentation_recovery.empty()) {
    return 47;
  }
  if (line_count(swapchain_recovery_policy_header) > 60 ||
      line_count(swapchain_recovery_policy) > 70 ||
      line_count(swapchain_recovery) > 40 ||
      line_count(swapchain_recovery_test) > 230 ||
      !contains(swapchain_recovery_policy_header,
                "struct VulkanSwapchainRecoveryPlan") ||
      !contains(swapchain_recovery_policy,
                "vulkan_plan_swapchain_recovery(") ||
      !contains(swapchain_recovery,
                "recover_swapchain_after_surface_status(") ||
      !contains(swapchain_recovery,
                "resize(descriptor_.framebuffer_size") ||
      contains(swapchain_recovery_policy, "vkAcquireNextImageKHR") ||
      contains(swapchain_recovery_policy, "vkQueuePresentKHR") ||
      contains(swapchain_recovery, "vkCreateSwapchainKHR")) {
    return 49;
  }
  if (line_count(presentation) > 165 ||
      !contains(presentation, "VulkanRendererState::present_frame(") ||
      !contains(presentation, "command_reuse_states_[image_index]") ||
      !contains(presentation, "vulkan_plan_swapchain_recovery(") ||
      !contains(presentation,
                "recover_swapchain_after_surface_status()") ||
      !contains(presentation, "vkAcquireNextImageKHR") ||
      !contains(presentation, "submit_frame(command_buffer)") ||
      !contains(presentation, "vkQueuePresentKHR") ||
      contains(presentation, "vkQueueSubmit") ||
      contains(presentation,
               "Result<void> VulkanRendererState::recover_after_failed_submit(") ||
      contains(presentation,
               "Result<void> VulkanRendererState::recover_after_failed_acquire(") ||
      contains(presentation,
               "Result<void> VulkanRendererState::recover_after_failed_present(") ||
      contains(presentation, "same_signature(") ||
      contains(presentation, "copy_signature(") ||
      contains(presentation, "VulkanRendererState::create(") ||
      contains(presentation, "vkCreateSwapchainKHR")) {
    return 32;
  }
  if (line_count(presentation_recovery) > 70 ||
      !contains(presentation_recovery,
                "VulkanRendererState::recover_after_failed_submit(") ||
      !contains(presentation_recovery,
                "VulkanRendererState::recover_after_failed_acquire(") ||
      !contains(presentation_recovery,
                "VulkanRendererState::recover_after_failed_present(") ||
      contains(presentation_recovery, "VulkanRendererState::present_frame(") ||
      contains(presentation_recovery, "vkAcquireNextImageKHR") ||
      contains(presentation_recovery, "vkQueuePresentKHR")) {
    return 48;
  }

  const std::string sync =
      read_source("src/renderer/vulkan/vulkan_sync.cpp");
  if (line_count(sync) > 120 ||
      !contains(sync, "VulkanRendererState::create_command_pool(") ||
      !contains(sync, "VulkanRendererState::destroy_sync_objects(") ||
      !contains(sync, "VulkanRendererState::create_sync_objects(") ||
      !contains(sync, "vkCreateCommandPool") ||
      !contains(sync, "vkCreateSemaphore") ||
      !contains(sync, "vkCreateFence") ||
      contains(sync, "vkQueuePresentKHR") ||
      contains(sync, "VulkanRendererState::present_frame(")) {
    return 33;
  }

  const std::string device =
      read_source("src/renderer/vulkan/vulkan_device.cpp");
  if (line_count(device) > 100) {
    return 13;
  }
  if (!contains(device, "VulkanRendererState::create_device(") ||
      !contains(device, "vkCreateDevice") ||
      contains(device, "VulkanRendererState::create_instance(") ||
      contains(device, "VulkanRendererState::create_surface(") ||
      contains(device, "VulkanRendererState::select_physical_device(") ||
      contains(device, "vkCreateInstance") ||
      contains(device, "vkEnumeratePhysicalDevices")) {
    return 14;
  }

  const std::string instance_surface =
      read_source("src/renderer/vulkan/vulkan_instance_surface.cpp");
  if (line_count(instance_surface) > 120 ||
      !contains(instance_surface,
                "VulkanRendererState::create_instance(") ||
      !contains(instance_surface, "VulkanRendererState::create_surface(") ||
      !contains(instance_surface, "vkCreateInstance") ||
      !contains(instance_surface, "vkCreateWin32SurfaceKHR") ||
      !contains(instance_surface, "vkCreateWaylandSurfaceKHR") ||
      contains(instance_surface, "vkCreateDevice")) {
    return 37;
  }

  const std::string physical_device =
      read_source("src/renderer/vulkan/vulkan_physical_device.cpp");
  if (line_count(physical_device) > 200 ||
      !contains(physical_device,
                "VulkanRendererState::find_queue_families(") ||
      !contains(physical_device,
                "VulkanRendererState::has_device_extension(") ||
      !contains(physical_device,
                "VulkanRendererState::select_physical_device(") ||
      !contains(physical_device, "vkEnumeratePhysicalDevices") ||
      !contains(physical_device, "vkGetPhysicalDeviceSurfaceSupportKHR") ||
      contains(physical_device, "vkCreateDevice") ||
      contains(physical_device, "vkCreateInstance")) {
    return 38;
  }

  const std::string swapchain =
      read_source("src/renderer/vulkan/vulkan_swapchain.cpp");
  const std::string swapchain_create =
      read_source("src/renderer/vulkan/vulkan_swapchain_create.cpp");
  const std::string swapchain_query =
      read_source("src/renderer/vulkan/vulkan_swapchain_query.cpp");
  const std::string swapchain_create_info =
      read_source("src/renderer/vulkan/vulkan_swapchain_create_info.cpp");
  if (swapchain_create.empty()) {
    return 49;
  }
  if (line_count(swapchain) > 20 ||
      contains(swapchain, "VulkanRendererState::create_swapchain_resources(") ||
      contains(swapchain, "vkCreateSwapchainKHR")) {
    return 15;
  }
  if (line_count(swapchain_create) > 100 ||
      !contains(swapchain_create,
                "VulkanRendererState::create_swapchain_resources(") ||
      !contains(swapchain_create, "query_vulkan_swapchain_surface(") ||
      !contains(swapchain_create, "build_vulkan_swapchain_create_plan(") ||
      !contains(swapchain_create, "make_vulkan_swapchain_create_info(") ||
      !contains(swapchain_create, "vkCreateSwapchainKHR") ||
      !contains(swapchain_query,
                "vkGetPhysicalDeviceSurfaceCapabilitiesKHR") ||
      !contains(swapchain_query,
                "vkGetPhysicalDeviceSurfaceFormatsKHR") ||
      !contains(swapchain_query,
                "vkGetPhysicalDeviceSurfacePresentModesKHR") ||
      !contains(swapchain_create_info,
                "build_vulkan_swapchain_create_plan(") ||
      !contains(swapchain_create_info,
                "make_vulkan_swapchain_create_info(") ||
      contains(swapchain_create,
               "vkGetPhysicalDeviceSurfaceFormatsKHR") ||
      contains(swapchain_create,
               "vkGetPhysicalDeviceSurfacePresentModesKHR") ||
      contains(swapchain_create,
               "VulkanRendererState::destroy_swapchain_resources(") ||
      contains(swapchain_create, "VulkanRendererState::install_swapchain(") ||
      contains(swapchain_create, "vkCreateImageView") ||
      contains(swapchain_create, "vkCreateRenderPass") ||
      contains(swapchain_create, "vkCreateFramebuffer") ||
      contains(swapchain_create, "vkAllocateCommandBuffers") ||
      contains(swapchain_create, "vkQueuePresentKHR")) {
    return 16;
  }

  const std::string swapchain_lifecycle =
      read_source("src/renderer/vulkan/vulkan_swapchain_lifecycle.cpp");
  if (line_count(swapchain_lifecycle) > 120 ||
      !contains(swapchain_lifecycle,
                "VulkanRendererState::destroy_swapchain_resources(") ||
      !contains(swapchain_lifecycle,
                "VulkanRendererState::destroy_swapchain()") ||
      !contains(swapchain_lifecycle,
                "VulkanRendererState::install_swapchain(") ||
      !contains(swapchain_lifecycle,
                "VulkanRendererState::create_swapchain()") ||
      contains(swapchain_lifecycle, "vkCreateSwapchainKHR") ||
      contains(swapchain_lifecycle, "vkCreateImageView")) {
    return 30;
  }

  const std::string swapchain_images =
      read_source("src/renderer/vulkan/vulkan_swapchain_images.cpp");
  if (line_count(swapchain_images) > 180 ||
      !contains(swapchain_images,
                "VulkanRendererState::populate_swapchain_images(") ||
      !contains(swapchain_images,
                "VulkanRendererState::create_swapchain_image_views(") ||
      !contains(swapchain_images, "vkGetSwapchainImagesKHR") ||
      !contains(swapchain_images, "vkCreateImageView") ||
      contains(swapchain_images, "vkCreateRenderPass") ||
      contains(swapchain_images, "vkAllocateCommandBuffers")) {
    return 27;
  }

  const std::string swapchain_render_pass =
      read_source("src/renderer/vulkan/vulkan_swapchain_render_pass.cpp");
  if (line_count(swapchain_render_pass) > 180 ||
      !contains(swapchain_render_pass,
                "VulkanRendererState::create_swapchain_render_pass(") ||
      !contains(swapchain_render_pass,
                "VulkanRendererState::create_swapchain_framebuffers(") ||
      !contains(swapchain_render_pass, "vkCreateRenderPass") ||
      !contains(swapchain_render_pass, "vkCreateFramebuffer") ||
      contains(swapchain_render_pass, "vkGetSwapchainImagesKHR") ||
      contains(swapchain_render_pass, "vkAllocateCommandBuffers")) {
    return 28;
  }

  const std::string swapchain_commands =
      read_source("src/renderer/vulkan/vulkan_swapchain_commands.cpp");
  if (line_count(swapchain_commands) > 120 ||
      !contains(swapchain_commands,
                "VulkanRendererState::allocate_swapchain_command_buffers(") ||
      !contains(swapchain_commands, "vkAllocateCommandBuffers") ||
      contains(swapchain_commands, "vkCreateFramebuffer") ||
      contains(swapchain_commands, "vkGetSwapchainImagesKHR")) {
    return 29;
  }

  const std::string report =
      read_source("src/renderer/vulkan/vulkan_report.cpp");
  if (line_count(report) > 150) {
    return 5;
  }
  if (!contains(report, "RendererCommandReport vulkan_build_renderer_command_report(") ||
      !contains(report, "vulkan_build_renderer_command_stream(") ||
      !contains(report, "vulkan_record_renderer_text_statistics(") ||
      !contains(report, "vulkan_record_renderer_image_statistics(") ||
      !contains(report, "vulkan_record_submission_plan_statistics(") ||
      contains(report, "RendererFrameReport vulkan_build_renderer_frame_report(") ||
      contains(report, "for (std::size_t index = 0; index < rects.size()") ||
      contains(report, "vulkan_build_textured_glyph_quads(") ||
      contains(report, "std::vector<GlyphAtlasUploadBatch> vulkan_plan_glyph_atlas_uploads(") ||
      contains(report, "std::vector<TexturedGlyphQuad> vulkan_build_textured_glyph_quads(") ||
      contains(report, "std::vector<RoundedRectTessellationRecord> vulkan_tessellate_rounded_rects(") ||
      contains(report, "std::vector<RendererSubmissionPlanRecord> vulkan_build_renderer_submission_plan(")) {
    return 17;
  }

  const std::string report_internal =
      read_source("src/renderer/vulkan/vulkan_report_internal.hpp");
  if (!contains(report_internal, "struct VulkanTextDrawAtlasPageUsage") ||
      !contains(report_internal, "vulkan_build_renderer_command_stream(") ||
      !contains(report_internal, "vulkan_record_renderer_text_statistics(") ||
      !contains(report_internal, "vulkan_same_command_batch_key(") ||
      !contains(report_internal, "vulkan_same_submission_plan_key(") ||
      !contains(report_internal, "vulkan_build_renderer_submission_plan(") ||
      !contains(report_internal, "vulkan_record_submission_plan_statistics(")) {
    return 18;
  }

  const std::string report_keys =
      read_source("src/renderer/vulkan/vulkan_report_keys.cpp");
  if (line_count(report_keys) > 90 ||
      !contains(report_keys, "vulkan_same_rect(") ||
      !contains(report_keys, "vulkan_same_clip_rect(") ||
      !contains(report_keys, "vulkan_same_clip_stack(") ||
      !contains(report_keys, "vulkan_same_command_batch_key(") ||
      !contains(report_keys, "vulkan_same_submission_plan_key(")) {
    return 46;
  }

  const std::string report_commands =
      read_source("src/renderer/vulkan/vulkan_report_commands.cpp");
  if (line_count(report_commands) > 140) {
    return 19;
  }
  if (!contains(report_commands, "append_command_batch(") ||
      !contains(report_commands, "vulkan_build_renderer_command_batches(") ||
      !contains(report_commands, "vulkan_build_renderer_command_report(") ||
      !contains(report_commands, "vulkan_record_submission_plan_statistics(") ||
      contains(report_commands, "same_rect(") ||
      contains(report_commands, "same_clip_stack(") ||
      contains(report_commands, "for (std::size_t index = 0; index < rects.size()") ||
      contains(report_commands, "append_submission_plan_record(") ||
      contains(report_commands, "vulkan_build_textured_glyph_quads(")) {
    return 20;
  }

  const std::string report_stream =
      read_source("src/renderer/vulkan/vulkan_report_stream.cpp");
  if (line_count(report_stream) > 170 ||
      !contains(report_stream, "vulkan_build_renderer_command_stream(") ||
      !contains(report_stream, "RendererCommandStreamItem") ||
      !contains(report_stream, "RendererPrimitiveKind::solid_rect") ||
      !contains(report_stream, "RendererPrimitiveKind::image") ||
      contains(report_stream, "vulkan_build_renderer_submission_plan(") ||
      contains(report_stream, "vulkan_build_textured_glyph_quads(")) {
    return 43;
  }

  const std::string report_payloads =
      read_source("src/renderer/vulkan/vulkan_report_payloads.cpp");
  if (line_count(report_payloads) > 120 ||
      !contains(report_payloads, "vulkan_record_renderer_text_statistics(") ||
      !contains(report_payloads, "vulkan_record_renderer_image_statistics(") ||
      !contains(report_payloads, "vulkan_build_textured_glyph_quads(") ||
      contains(report_payloads, "vulkan_build_renderer_submission_plan(") ||
      contains(report_payloads, "RendererFrameReport vulkan_build_renderer_frame_report(")) {
    return 44;
  }

  const std::string report_frame =
      read_source("src/renderer/vulkan/vulkan_report_frame.cpp");
  if (line_count(report_frame) > 80 ||
      !contains(report_frame, "RendererFrameReport vulkan_build_renderer_frame_report(") ||
      !contains(report_frame, "renderer_frame_report_from_command_report(") ||
      contains(report_frame, "vulkan_build_textured_glyph_quads(") ||
      contains(report_frame, "vulkan_record_submission_plan_statistics(")) {
    return 45;
  }

  const std::string report_submission =
      read_source("src/renderer/vulkan/vulkan_report_submission.cpp");
  const std::string report_text_submission =
      read_source("src/renderer/vulkan/vulkan_report_text_submission.cpp");
  const std::string report_submission_stats =
      read_source("src/renderer/vulkan/vulkan_report_submission_stats.cpp");
  if (line_count(report_submission) > 100) {
    return 25;
  }
  if (!contains(report_text_submission,
                "vulkan_append_text_draw_atlas_page_usage(") ||
      !contains(report_text_submission,
                "VulkanTextDrawAtlasPageUsage") ||
      !contains(report_submission_stats,
                "vulkan_record_submission_plan_statistics(") ||
      !contains(report_submission, "append_submission_plan_record(") ||
      !contains(report_submission, "vulkan_build_renderer_submission_plan(") ||
      contains(report_submission,
               "vulkan_append_text_draw_atlas_page_usage(") ||
      contains(report_submission,
               "vulkan_record_submission_plan_statistics(") ||
      contains(report_submission, "same_rect(") ||
      contains(report_submission, "same_clip_stack(") ||
      contains(report_submission, "bool same_submission_plan_key(") ||
      contains(report_submission, "vulkan_build_renderer_command_report(")) {
    return 26;
  }

  const std::string report_uploads =
      read_source("src/renderer/vulkan/vulkan_report_uploads.cpp");
  if (line_count(report_uploads) > 40 ||
      contains(report_uploads, "vulkan_consume_text_draw(") ||
      contains(report_uploads, "vulkan_plan_glyph_atlas_uploads(") ||
      contains(report_uploads, "vulkan_plan_image_uploads(") ||
      contains(report_uploads, "vulkan_update_glyph_atlas_texture_resources(") ||
      contains(report_uploads, "vulkan_plan_glyph_atlas_dirty_uploads(") ||
      contains(report_uploads, "vulkan_build_textured_glyph_quads(")) {
    return 21;
  }

  const std::string report_text_draws =
      read_source("src/renderer/vulkan/vulkan_report_text_draws.cpp");
  if (line_count(report_text_draws) > 60 ||
      !contains(report_text_draws, "vulkan_consume_text_draw(") ||
      !contains(report_text_draws, "vulkan_build_textured_glyph_quads(") ||
      contains(report_text_draws, "rasterize_fallback_glyph(") ||
      contains(report_text_draws, "vulkan_plan_glyph_atlas_uploads(")) {
    return 22;
  }

  const std::string report_glyph_uploads =
      read_source("src/renderer/vulkan/vulkan_report_glyph_uploads.cpp");
  if (line_count(report_glyph_uploads) > 120 ||
      !contains(report_glyph_uploads, "vulkan_plan_glyph_atlas_uploads(") ||
      !contains(report_glyph_uploads, "GlyphAtlasUploadRegion") ||
      contains(report_glyph_uploads, "vulkan_plan_image_uploads(") ||
      contains(report_glyph_uploads, "vulkan_build_textured_glyph_quads(")) {
    return 39;
  }

  const std::string report_texture_resources =
      read_source("src/renderer/vulkan/vulkan_report_texture_resources.cpp");
  const std::string glyph_atlas_production_source =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_production.cpp");
  const std::string glyph_atlas_resources_internal = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_resources_internal.hpp");
  const std::string glyph_atlas_descriptors = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_descriptors.cpp");
  const std::string glyph_atlas_images =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_images.cpp");
  const std::string glyph_atlas_resources =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_resources.cpp");
  const std::string glyph_atlas_draw_bindings_internal = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_draw_bindings_internal.hpp");
  const std::string glyph_atlas_draw_bindings = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_draw_bindings.cpp");
  const std::string glyph_atlas_draw_data =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_draw_data.cpp");
  const std::string glyph_atlas_multi_page_test =
      read_source("tests/renderer/vulkan_glyph_atlas_multi_page_test.cpp");
  const std::string glyph_atlas_uploads_internal = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_uploads_internal.hpp");
  const std::string command_recording_internal = read_source(
      "src/renderer/vulkan/vulkan_command_recording_internal.hpp");
  const std::string device_memory =
      read_source("src/renderer/vulkan/vulkan_device_memory.cpp");
  const std::string glyph_atlas_staging =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_staging.cpp");
  const std::string glyph_atlas_upload_recording = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_upload_recording.cpp");
  const std::string upload_barrier_batch_header = read_source(
      "src/renderer/vulkan/vulkan_upload_barrier_batch_internal.hpp");
  const std::string upload_barrier_batch = read_source(
      "src/renderer/vulkan/vulkan_upload_barrier_batch.cpp");
  const std::string upload_barrier_batch_test = read_source(
      "tests/renderer/vulkan_upload_barrier_batching_test.cpp");
  const std::string glyph_atlas_frame =
      read_source("src/renderer/vulkan/vulkan_glyph_atlas_frame.cpp");
  if (line_count(report_texture_resources) > 150 ||
      !contains(
          report_texture_resources,
          "vulkan_update_glyph_atlas_texture_resources(") ||
      !contains(
          report_texture_resources,
          "vulkan_plan_glyph_atlas_dirty_uploads(") ||
      !contains(report_texture_resources, "GlyphAtlasTextureResourceState") ||
      contains(report_texture_resources, "vulkan_plan_image_uploads(") ||
      contains(report_texture_resources, "vulkan_build_textured_glyph_quads(")) {
    return 40;
  }
  if (line_count(glyph_atlas_production_source) > 120 ||
      !contains(
          glyph_atlas_production_source,
          "vulkan_plan_glyph_atlas_production_resources(") ||
      !contains(glyph_atlas_production_source,
                "GlyphAtlasProductionResourceState") ||
      !contains(glyph_atlas_production_source,
                "GlyphAtlasProductionUploadCommand") ||
      contains(glyph_atlas_production_source,
               "vulkan_update_glyph_atlas_texture_resources(") ||
      contains(glyph_atlas_production_source,
               "vulkan_build_textured_glyph_quads(")) {
    return 58;
  }
  if (line_count(glyph_atlas_resources_internal) > 100 ||
      line_count(glyph_atlas_descriptors) > 130 ||
      line_count(glyph_atlas_images) > 220 ||
      line_count(glyph_atlas_resources) > 150 ||
      line_count(glyph_atlas_draw_bindings_internal) > 70 ||
      line_count(glyph_atlas_draw_bindings) > 120 ||
      line_count(glyph_atlas_draw_data) > 90 ||
      line_count(glyph_atlas_multi_page_test) > 260 ||
      !contains(glyph_atlas_resources_internal,
                "struct VulkanGlyphAtlasResources") ||
      !contains(glyph_atlas_resources_internal,
                "vulkan_glyph_atlas_descriptor_capacity") ||
      !contains(glyph_atlas_descriptors, "vkCreateDescriptorSetLayout") ||
      !contains(glyph_atlas_descriptors,
                "vulkan_glyph_atlas_descriptor_capacity") ||
      !contains(glyph_atlas_images, "vkCreateImage") ||
      !contains(glyph_atlas_resources,
                "vulkan_update_glyph_atlas_resources(") ||
      !contains(glyph_atlas_resources,
                "vulkan_glyph_atlas_plan_fits_descriptor_capacity(plan)") ||
      !contains(glyph_atlas_draw_bindings_internal,
                "struct VulkanGlyphAtlasDrawBinding") ||
      !contains(glyph_atlas_draw_bindings,
                "vulkan_resolve_glyph_atlas_draw_bindings(") ||
      !contains(glyph_atlas_draw_bindings,
                "vulkan_validate_glyph_atlas_draw_bindings(") ||
      !contains(glyph_atlas_draw_data,
                "vulkan_plan_glyph_atlas_draw_data(") ||
      !contains(glyph_atlas_multi_page_test,
                "test_three_page_allocation_and_upload_planning(") ||
      !contains(glyph_atlas_multi_page_test,
                "test_descriptor_capacity_rejection_is_preflight(")) {
    return 59;
  }
  if (line_count(upload_barrier_batch_header) > 70 ||
      line_count(upload_barrier_batch) > 150 ||
      line_count(upload_barrier_batch_test) > 260 ||
      !contains(upload_barrier_batch_header,
                "struct VulkanUploadBarrierPlan") ||
      !contains(upload_barrier_batch,
                "vulkan_plan_upload_image_barriers(") ||
      !contains(upload_barrier_batch, "vkCmdPipelineBarrier") ||
      contains(upload_barrier_batch, "vkCmdCopyBufferToImage")) {
    return 57;
  }
  if (line_count(glyph_atlas_uploads_internal) > 80 ||
      line_count(command_recording_internal) > 40 ||
      line_count(device_memory) > 50 ||
      line_count(glyph_atlas_staging) > 230 ||
      line_count(glyph_atlas_upload_recording) > 170 ||
      line_count(glyph_atlas_frame) > 80 ||
      !contains(glyph_atlas_staging, "VK_BUFFER_USAGE_TRANSFER_SRC_BIT") ||
      !contains(glyph_atlas_upload_recording,
                "vulkan_plan_upload_image_barriers(") ||
      contains(glyph_atlas_upload_recording, "vkCmdPipelineBarrier") ||
      !contains(glyph_atlas_upload_recording, "vkCmdCopyBufferToImage") ||
      !contains(glyph_atlas_frame, "prepare_glyph_atlas_frame(")) {
    return 60;
  }

  const std::string text_pipeline_internal =
      read_source("src/renderer/vulkan/vulkan_text_pipeline_internal.hpp");
  const std::string text_pipeline_state =
      read_source("src/renderer/vulkan/vulkan_text_pipeline_state.cpp");
  if (line_count(text_pipeline_internal) > 70 ||
      line_count(text_pipeline_state) > 150 ||
      !contains(text_pipeline_internal, "struct VulkanTextVertex") ||
      !contains(text_pipeline_state,
                "vulkan_text_vertex_binding_description(") ||
      !contains(text_pipeline_state,
                "vulkan_text_pipeline_blend_attachment_state(") ||
      !contains(text_pipeline_state, "VK_DYNAMIC_STATE_VIEWPORT") ||
      !contains(text_pipeline_state, "VK_DYNAMIC_STATE_SCISSOR") ||
      contains(command_recording,
               "vulkan_text_pipeline_input_assembly_state(")) {
    return 61;
  }

  const std::string text_shader_binaries =
      read_source("src/renderer/vulkan/vulkan_text_shader_binaries.cpp");
  const std::string text_shader_modules =
      read_source("src/renderer/vulkan/vulkan_text_shader_modules.cpp");
  const std::string text_vertex_shader =
      read_source("src/renderer/vulkan/shaders/text.vert.glsl");
  const std::string text_fragment_shader =
      read_source("src/renderer/vulkan/shaders/text.frag.glsl");
  if (line_count(text_shader_binaries) > 130 ||
      line_count(text_shader_modules) > 100 ||
      line_count(text_vertex_shader) > 60 ||
      line_count(text_fragment_shader) > 40 ||
      !contains(text_shader_binaries,
                "vulkan_text_vertex_shader_spirv(") ||
      !contains(text_shader_binaries,
                "vulkan_text_fragment_shader_spirv(") ||
      !contains(text_shader_modules, "vkCreateShaderModule") ||
      !contains(text_shader_modules, "vkDestroyShaderModule") ||
      !contains(text_vertex_shader, "layout(push_constant)") ||
      !contains(text_fragment_shader,
                "layout(set = 0, binding = 0)") ||
      contains(command_recording, "vkCreateShaderModule")) {
    return 62;
  }

  const std::string text_pipeline_resources_header = read_source(
      "src/renderer/vulkan/vulkan_text_pipeline_resources_internal.hpp");
  const std::string text_pipeline_resources = read_source(
      "src/renderer/vulkan/vulkan_text_pipeline_resources.cpp");
  if (line_count(text_pipeline_resources_header) > 60 ||
      line_count(text_pipeline_resources) > 190 ||
      !contains(text_pipeline_resources_header,
                "struct VulkanTextPipelineResources") ||
      !contains(text_pipeline_resources, "vkCreatePipelineLayout") ||
      !contains(text_pipeline_resources, "vkCreateGraphicsPipelines") ||
      !contains(text_pipeline_resources, "vkDestroyPipeline") ||
      contains(command_recording, "vkCreateGraphicsPipelines")) {
    return 63;
  }

  const std::string text_vertex_buffer_header = read_source(
      "src/renderer/vulkan/vulkan_text_vertex_buffer_internal.hpp");
  const std::string text_vertex_buffer =
      read_source("src/renderer/vulkan/vulkan_text_vertex_buffer.cpp");
  const std::string frame_geometry_buffer_header = read_source(
      "src/renderer/vulkan/vulkan_frame_geometry_buffer_internal.hpp");
  const std::string frame_geometry_buffer = read_source(
      "src/renderer/vulkan/vulkan_frame_geometry_buffer.cpp");
  if (line_count(frame_geometry_buffer_header) > 80 ||
      line_count(frame_geometry_buffer) > 210 ||
      !contains(frame_geometry_buffer_header,
                "struct VulkanFrameGeometryBufferResources") ||
      !contains(frame_geometry_buffer,
                "vulkan_plan_frame_geometry_buffer(") ||
      !contains(frame_geometry_buffer,
                "vulkan_upload_frame_geometry_buffer(") ||
      !contains(frame_geometry_buffer, "vkMapMemory") ||
      !contains(frame_geometry_buffer, "vkCreateBuffer")) {
    return 64;
  }
  if (line_count(text_vertex_buffer_header) > 60 ||
      line_count(text_vertex_buffer) > 190 ||
      !contains(text_vertex_buffer_header,
                "struct VulkanTextVertexBufferResources") ||
      !contains(text_vertex_buffer_header,
                "VulkanFrameGeometryBufferResources vertices") ||
      !contains(text_vertex_buffer, "vulkan_build_text_vertices(") ||
      !contains(text_vertex_buffer, "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT") ||
      !contains(text_vertex_buffer,
                "vulkan_upload_frame_geometry_buffer(") ||
      !contains(text_vertex_buffer, "vulkan_apply_composed_transform(") ||
      contains(command_recording, "vulkan_build_text_vertices(")) {
    return 65;
  }

  const std::string text_draw_recording_header = read_source(
      "src/renderer/vulkan/vulkan_text_draw_recording_internal.hpp");
  const std::string text_draw_recording =
      read_source("src/renderer/vulkan/vulkan_text_draw_recording.cpp");
  if (line_count(text_draw_recording_header) > 60 ||
      line_count(text_draw_recording) > 170 ||
      !contains(text_draw_recording, "vulkan_plan_text_draw_commands(") ||
      !contains(text_draw_recording, "vkCmdBindPipeline") ||
      !contains(text_draw_recording, "vkCmdBindDescriptorSets") ||
      !contains(text_draw_recording, "vkCmdDraw") ||
      contains(command_recording, "vulkan_record_text_draws(") ||
      contains(command_recording, "vkCmdDraw")) {
    return 65;
  }

  const std::string text_positioning_header = read_source(
      "src/renderer/vulkan/vulkan_text_positioning_internal.hpp");
  const std::string text_positioning =
      read_source("src/renderer/vulkan/vulkan_text_positioning.cpp");
  if (line_count(text_positioning_header) > 40 ||
      line_count(text_positioning) > 60 ||
      !contains(text_positioning_header,
                "enum class VulkanTextPositioningPolicy") ||
      !contains(text_positioning, "preserve_subpixel") ||
      !contains(text_positioning, "std::round") ||
      contains(command_recording, "VulkanTextPositioningPolicy")) {
    return 66;
  }

  const std::string text_coverage_header = read_source(
      "src/renderer/vulkan/vulkan_text_coverage_internal.hpp");
  const std::string text_coverage =
      read_source("src/renderer/vulkan/vulkan_text_coverage.cpp");
  if (line_count(text_coverage_header) > 50 ||
      line_count(text_coverage) > 60 ||
      !contains(text_coverage_header, "struct VulkanTextCoveragePolicy") ||
      !contains(text_coverage, "vulkan_resolve_text_coverage(") ||
      !contains(text_coverage, "std::pow") ||
      contains(command_recording, "VulkanTextCoveragePolicy")) {
    return 67;
  }

  const std::string rounded_rect_geometry_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_geometry_internal.hpp");
  const std::string rounded_rect_geometry =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_geometry.cpp");
  const std::string rounded_rect_vertex_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_vertex_internal.hpp");
  const std::string rounded_rect_contour_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_contour_internal.hpp");
  const std::string rounded_rect_contour =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_contour.cpp");
  const std::string rounded_rect_indices_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_indices_internal.hpp");
  const std::string rounded_rect_indices =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_indices.cpp");
  const std::string rounded_rect_antialiasing_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_antialiasing_internal.hpp");
  const std::string rounded_rect_antialiasing = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_antialiasing.cpp");
  const std::string rounded_rect_radii_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_radii_internal.hpp");
  const std::string rounded_rect_radii = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_radii.cpp");
  const std::string rounded_rect_stroke_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_stroke_internal.hpp");
  const std::string rounded_rect_stroke =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_stroke.cpp");
  if (line_count(rounded_rect_geometry_header) > 80 ||
      line_count(rounded_rect_geometry) > 190 ||
      line_count(rounded_rect_vertex_header) > 30 ||
      line_count(rounded_rect_contour_header) > 50 ||
      line_count(rounded_rect_contour) > 120 ||
      line_count(rounded_rect_indices_header) > 40 ||
      line_count(rounded_rect_indices) > 70 ||
      line_count(rounded_rect_antialiasing_header) > 50 ||
      line_count(rounded_rect_antialiasing) > 50 ||
      line_count(rounded_rect_radii_header) > 40 ||
      line_count(rounded_rect_radii) > 90 ||
      line_count(rounded_rect_stroke_header) > 50 ||
      line_count(rounded_rect_stroke) > 80 ||
      !contains(rounded_rect_geometry_header,
                "struct VulkanRoundedRectGeometry") ||
      !contains(rounded_rect_vertex_header,
                "struct VulkanRoundedRectVertex") ||
      !contains(rounded_rect_contour,
                "append_corner_arc(") ||
      !contains(rounded_rect_contour,
                "vulkan_append_rounded_rect_contour(") ||
      !contains(rounded_rect_indices,
                "vulkan_append_rounded_rect_fan_indices(") ||
      !contains(rounded_rect_indices,
                "vulkan_append_rounded_rect_ring_indices(") ||
      !contains(rounded_rect_geometry,
                "vulkan_build_rounded_rect_geometry(") ||
      !contains(rounded_rect_geometry,
                "vulkan_append_rounded_rect_contour(") ||
      !contains(rounded_rect_geometry,
                "vulkan_append_rounded_rect_fan_indices(") ||
      !contains(rounded_rect_geometry,
                "vulkan_append_rounded_rect_ring_indices(") ||
      !contains(rounded_rect_antialiasing_header,
                "struct VulkanRoundedRectAntialiasingPolicy") ||
      !contains(rounded_rect_antialiasing,
                "vulkan_rounded_rect_coverage(") ||
      !contains(rounded_rect_radii_header,
                "struct VulkanRoundedRectRadiiResolution") ||
      !contains(rounded_rect_radii,
                "vulkan_resolve_rounded_rect_radii(") ||
      !contains(rounded_rect_geometry,
                "vulkan_resolve_rounded_rect_radii(") ||
      !contains(rounded_rect_stroke_header,
                "struct VulkanRoundedRectStrokeResolution") ||
      !contains(rounded_rect_stroke,
                "vulkan_resolve_rounded_rect_stroke(") ||
      !contains(rounded_rect_stroke,
                "vulkan_resolve_rounded_rect_radii(") ||
      !contains(rounded_rect_geometry,
                "vulkan_resolve_rounded_rect_stroke(") ||
      !contains(rounded_rect_geometry,
                "vulkan_apply_composed_transform(") ||
      contains(rounded_rect_geometry, "indices.insert(") ||
      contains(rounded_rect_geometry, "append_corner_arc(") ||
      contains(command_recording, "append_corner_arc(")) {
    return 68;
  }

  const std::string rounded_rect_buffers_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_buffers_internal.hpp");
  const std::string rounded_rect_buffers =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_buffers.cpp");
  const std::string rounded_rect_frame =
      read_source("src/renderer/vulkan/vulkan_rounded_rect_frame.cpp");
  if (line_count(rounded_rect_buffers_header) > 70 ||
      line_count(rounded_rect_buffers) > 190 ||
      line_count(rounded_rect_frame) > 40 ||
      !contains(rounded_rect_buffers_header,
                "struct VulkanRoundedRectBufferResources") ||
      !contains(rounded_rect_buffers_header,
                "VulkanFrameGeometryBufferResources vertices") ||
      !contains(rounded_rect_buffers_header,
                "VulkanFrameGeometryBufferResources indices") ||
      !contains(rounded_rect_buffers,
                "vulkan_upload_rounded_rect_buffers(") ||
      !contains(rounded_rect_buffers,
                "vulkan_upload_frame_geometry_buffer(") ||
      !contains(rounded_rect_buffers, "VK_BUFFER_USAGE_INDEX_BUFFER_BIT") ||
      !contains(rounded_rect_frame, "prepare_rounded_rect_frame(")) {
    return 69;
  }

  const std::string rounded_rect_pipeline_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_pipeline_internal.hpp");
  const std::string rounded_rect_pipeline_state = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_pipeline_state.cpp");
  const std::string rounded_rect_shader_binaries = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_shader_binaries.cpp");
  const std::string rounded_rect_shader_modules = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_shader_modules.cpp");
  const std::string rounded_rect_pipeline_resources = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_pipeline_resources.cpp");
  if (line_count(rounded_rect_pipeline_header) > 110 ||
      line_count(rounded_rect_pipeline_state) > 140 ||
      line_count(rounded_rect_shader_binaries) > 120 ||
      line_count(rounded_rect_shader_modules) > 110 ||
      line_count(rounded_rect_pipeline_resources) > 190 ||
      !contains(rounded_rect_pipeline_header,
                "struct VulkanRoundedRectPipelineResources") ||
      !contains(rounded_rect_pipeline_state,
                "vulkan_rounded_rect_vertex_binding_description(") ||
      !contains(rounded_rect_pipeline_state,
                "offsetof(VulkanRoundedRectVertex, coverage)") ||
      !contains(rounded_rect_shader_binaries,
                "vulkan_rounded_rect_vertex_shader_spirv(") ||
      !contains(rounded_rect_shader_modules, "vkCreateShaderModule") ||
      !contains(rounded_rect_pipeline_resources,
                "vkCreateGraphicsPipelines")) {
    return 70;
  }

  const std::string rounded_rect_draw_recording_header = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_draw_recording_internal.hpp");
  const std::string rounded_rect_draw_recording = read_source(
      "src/renderer/vulkan/vulkan_rounded_rect_draw_recording.cpp");
  if (line_count(rounded_rect_draw_recording_header) > 50 ||
      line_count(rounded_rect_draw_recording) > 150 ||
      !contains(rounded_rect_draw_recording,
                "vulkan_validate_rounded_rect_draw_resources(") ||
      !contains(rounded_rect_draw_recording, "vkCmdBindIndexBuffer") ||
      !contains(rounded_rect_draw_recording, "vkCmdDrawIndexed") ||
      contains(command_recording,
               "vulkan_record_rounded_rect_draws(") ||
      contains(command_recording, "vkCmdDrawIndexed")) {
    return 71;
  }

  const std::string report_image_uploads =
      read_source("src/renderer/vulkan/vulkan_report_image_uploads.cpp");
  if (line_count(report_image_uploads) > 80 ||
      !contains(report_image_uploads, "vulkan_plan_image_uploads(") ||
      !contains(report_image_uploads, "ImageUploadRegion") ||
      contains(report_image_uploads, "vulkan_plan_glyph_atlas_uploads(") ||
      contains(report_image_uploads, "vulkan_build_textured_glyph_quads(")) {
    return 41;
  }

  const std::string image_texture_header = read_source(
      "src/renderer/vulkan/vulkan_image_texture_resources_internal.hpp");
  const std::string image_texture_images = read_source(
      "src/renderer/vulkan/vulkan_image_texture_images.cpp");
  const std::string image_texture_resources = read_source(
      "src/renderer/vulkan/vulkan_image_texture_resources.cpp");
  const std::string image_texture_frame = read_source(
      "src/renderer/vulkan/vulkan_image_texture_frame.cpp");
  const std::string image_texture_cache_header = read_source(
      "src/renderer/vulkan/vulkan_image_texture_cache_internal.hpp");
  const std::string image_texture_cache = read_source(
      "src/renderer/vulkan/vulkan_image_texture_cache.cpp");
  const std::string image_texture_invalidation_header = read_source(
      "src/renderer/vulkan/vulkan_image_texture_invalidation_internal.hpp");
  const std::string image_texture_invalidation = read_source(
      "src/renderer/vulkan/vulkan_image_texture_invalidation.cpp");
  const std::string image_texture_upload_header = read_source(
      "src/renderer/vulkan/vulkan_image_texture_uploads_internal.hpp");
  const std::string image_texture_staging = read_source(
      "src/renderer/vulkan/vulkan_image_texture_staging.cpp");
  const std::string image_texture_upload_recording = read_source(
      "src/renderer/vulkan/vulkan_image_texture_upload_recording.cpp");
  const std::string image_texture_state =
      read_source("src/renderer/vulkan/vulkan_state_internal.hpp");
  const std::string image_texture_presentation =
      read_source("src/renderer/vulkan/vulkan_presentation.cpp");
  if (line_count(image_texture_header) > 90 ||
      line_count(image_texture_images) > 180 ||
      line_count(image_texture_resources) > 160 ||
      line_count(image_texture_frame) > 110 ||
      line_count(image_texture_cache_header) > 60 ||
      line_count(image_texture_cache) > 110 ||
      line_count(image_texture_invalidation_header) > 40 ||
      line_count(image_texture_invalidation) > 80 ||
      !contains(image_texture_header, "struct VulkanImageTextureResource") ||
      !contains(image_texture_header, "struct VulkanImageTextureResources") ||
      !contains(image_texture_images, "vkCreateImage") ||
      !contains(image_texture_images, "vkCreateImageView") ||
      !contains(image_texture_resources,
                "vulkan_update_image_texture_resources(") ||
      !contains(image_texture_frame, "prepare_image_texture_frame(") ||
      !contains(image_texture_frame,
                "vulkan_prepare_image_texture_cache_frame(") ||
      !contains(image_texture_cache_header,
                "vulkan_image_texture_cache_max_idle_frames") ||
      !contains(image_texture_cache,
                "vulkan_evict_idle_image_texture_resources(") ||
      !contains(image_texture_invalidation,
                "vulkan_destroy_image_texture_resource(") ||
      !contains(image_texture_state,
                "VulkanImageTextureResources image_texture_resources_") ||
      !contains(image_texture_presentation,
                "image_draws, image_uploads, image_invalidations")) {
    return 72;
  }
  if (line_count(image_texture_upload_header) > 100 ||
      line_count(image_texture_staging) > 190 ||
      line_count(image_texture_upload_recording) > 150 ||
      !contains(image_texture_upload_header,
                "struct VulkanImageTextureStagingUpload") ||
      !contains(image_texture_upload_header,
                "struct VulkanImageTextureUploadResources") ||
      !contains(image_texture_staging, "VK_BUFFER_USAGE_TRANSFER_SRC_BIT") ||
      !contains(image_texture_staging,
                "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT") ||
      !contains(image_texture_staging, "vkMapMemory") ||
      !contains(image_texture_upload_recording,
                "vulkan_plan_upload_image_barriers(") ||
      contains(image_texture_upload_recording, "vkCmdPipelineBarrier") ||
      !contains(image_texture_upload_recording,
                "vkCmdCopyBufferToImage") ||
      !contains(image_texture_upload_recording,
                "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL") ||
      !contains(image_texture_frame, "vulkan_stage_image_texture_uploads(") ||
      !contains(image_texture_state,
                "VulkanImageTextureUploadResources image_texture_uploads_")) {
    return 73;
  }
  const std::string image_sampling =
      read_source("include/cgpui/renderer/image_sampling.hpp");
  const std::string image_commands =
      read_source("include/cgpui/renderer/renderer_commands.hpp");
  const std::string image_texture_descriptor_header = read_source(
      "src/renderer/vulkan/vulkan_image_texture_descriptors_internal.hpp");
  const std::string image_texture_samplers = read_source(
      "src/renderer/vulkan/vulkan_image_texture_samplers.cpp");
  const std::string image_texture_descriptors = read_source(
      "src/renderer/vulkan/vulkan_image_texture_descriptors.cpp");
  const std::string image_texture_resource_binding = read_source(
      "src/renderer/vulkan/vulkan_image_texture_resource_binding.cpp");
  if (line_count(image_sampling) > 40 ||
      line_count(image_texture_descriptor_header) > 100 ||
      line_count(image_texture_samplers) > 80 ||
      line_count(image_texture_descriptors) > 220 ||
      line_count(image_texture_resource_binding) > 100 ||
      !contains(image_sampling, "enum class ImageSamplingMode") ||
      !contains(image_commands, "ImageSamplingMode sampling") ||
      !contains(image_texture_descriptor_header,
                "vulkan_image_texture_descriptor_capacity = 256") ||
      !contains(image_texture_samplers, "vkCreateSampler") ||
      !contains(image_texture_descriptors, "vkCreateDescriptorSetLayout") ||
      !contains(image_texture_descriptors, "vkAllocateDescriptorSets") ||
      !contains(image_texture_resource_binding,
                "vulkan_bind_image_texture_resource_descriptors(")) {
    return 74;
  }
  const std::string image_vertex_shader =
      read_source("src/renderer/vulkan/shaders/image.vert.glsl");
  const std::string image_fragment_shader =
      read_source("src/renderer/vulkan/shaders/image.frag.glsl");
  const std::string image_color_header = read_source(
      "src/renderer/vulkan/vulkan_image_color_internal.hpp");
  const std::string image_color =
      read_source("src/renderer/vulkan/vulkan_image_color.cpp");
  const std::string image_pipeline_header = read_source(
      "src/renderer/vulkan/vulkan_image_pipeline_internal.hpp");
  const std::string image_pipeline_resources_header = read_source(
      "src/renderer/vulkan/vulkan_image_pipeline_resources_internal.hpp");
  const std::string image_pipeline_state = read_source(
      "src/renderer/vulkan/vulkan_image_pipeline_state.cpp");
  const std::string image_pipeline_resources = read_source(
      "src/renderer/vulkan/vulkan_image_pipeline_resources.cpp");
  const std::string image_shader_binaries = read_source(
      "src/renderer/vulkan/vulkan_image_shader_binaries.cpp");
  const std::string image_shader_modules = read_source(
      "src/renderer/vulkan/vulkan_image_shader_modules.cpp");
  const std::string image_vertex_buffer_header = read_source(
      "src/renderer/vulkan/vulkan_image_vertex_buffer_internal.hpp");
  const std::string image_vertex_buffer = read_source(
      "src/renderer/vulkan/vulkan_image_vertex_buffer.cpp");
  const std::string image_draw_recording_header = read_source(
      "src/renderer/vulkan/vulkan_image_draw_recording_internal.hpp");
  const std::string image_draw_recording = read_source(
      "src/renderer/vulkan/vulkan_image_draw_recording.cpp");
  const std::string image_frame_order =
      read_source("src/renderer/vulkan/vulkan_frame_draw_order.cpp");
  const std::string image_frame_recording =
      read_source("src/renderer/vulkan/vulkan_frame_draw_recording.cpp");
  const std::string image_command_recording =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  const std::string image_swapchain_create =
      read_source("src/renderer/vulkan/vulkan_swapchain_create.cpp");
  const std::string image_swapchain_lifecycle =
      read_source("src/renderer/vulkan/vulkan_swapchain_lifecycle.cpp");
  if (line_count(image_color_header) > 30 ||
      line_count(image_color) > 50 ||
      line_count(image_pipeline_header) > 100 ||
      line_count(image_pipeline_resources_header) > 80 ||
      line_count(image_pipeline_state) > 140 ||
      line_count(image_pipeline_resources) > 190 ||
      line_count(image_shader_binaries) > 140 ||
      line_count(image_shader_modules) > 120 ||
      line_count(image_vertex_buffer_header) > 80 ||
      line_count(image_vertex_buffer) > 210 ||
      line_count(image_draw_recording_header) > 100 ||
      line_count(image_draw_recording) > 180 ||
      !contains(image_color_header, "vulkan_resolve_image_tint(") ||
      !contains(image_color, "vulkan_apply_composed_opacity(") ||
      !contains(image_vertex_shader, "out_color") ||
      !contains(image_fragment_shader,
                "texture(image_texture, in_image_uv) *") ||
      !contains(image_pipeline_header, "struct VulkanImageVertex") ||
      !contains(image_pipeline_resources,
                "vkCreateGraphicsPipelines") ||
      !contains(image_shader_binaries,
                "vulkan_image_fragment_shader_spirv") ||
      !contains(image_shader_modules, "vkCreateShaderModule") ||
      !contains(image_vertex_buffer,
                "vulkan_apply_composed_transform") ||
      !contains(image_vertex_buffer, "vulkan_resolve_image_tint(") ||
      !contains(image_vertex_buffer, "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT") ||
      !contains(image_vertex_buffer,
                "vulkan_upload_frame_geometry_buffer(") ||
      !contains(image_draw_recording,
                "vulkan_plan_image_draw_commands") ||
      !contains(image_draw_recording,
                "image_texture_shader_readable_for_frame") ||
      !contains(image_draw_recording, "vkCmdBindDescriptorSets") ||
      !contains(image_draw_recording, "vkCmdDraw") ||
      !contains(image_frame_order,
                "VulkanFrameDrawResourceKind::image") ||
      !contains(image_frame_recording, "vulkan_record_image_draw(") ||
      !contains(image_command_recording,
                "vulkan_plan_image_draw_commands(") ||
      !contains(image_swapchain_create,
                "vulkan_create_image_pipeline_resources(") ||
      !contains(image_swapchain_lifecycle,
                "vulkan_destroy_image_pipeline_resources(") ||
      !contains(image_texture_frame,
                "vulkan_upload_image_vertex_buffer(") ||
      !contains(image_texture_state,
                "VulkanImagePipelineResources image_pipeline_resources_") ||
      !contains(image_texture_state,
                "VulkanImageVertexBufferResources image_vertex_buffer_")) {
    return 75;
  }

  const std::string report_text_quads =
      read_source("src/renderer/vulkan/vulkan_report_text_quads.cpp");
  if (line_count(report_text_quads) > 140 ||
      !contains(report_text_quads, "vulkan_build_textured_glyph_quads(") ||
      !contains(report_text_quads, "normalized_atlas_bounds(") ||
      !contains(report_text_quads, "rasterize_fallback_glyph(") ||
      contains(report_text_quads, "vulkan_plan_glyph_atlas_uploads(") ||
      contains(report_text_quads, "vulkan_plan_image_uploads(")) {
    return 42;
  }

  const std::string report_geometry =
      read_source("src/renderer/vulkan/vulkan_report_geometry.cpp");
  if (line_count(report_geometry) > 260) {
    return 23;
  }
  if (!contains(report_geometry, "vulkan_tessellate_rounded_rects(") ||
      !contains(report_geometry,
                ".fill_enabled = rounded_rect.fill_enabled") ||
      !contains(report_geometry, "vulkan_build_text_selection_geometry(") ||
      !contains(report_geometry, "vulkan_build_text_caret_geometry(") ||
      contains(report_geometry, "vulkan_plan_glyph_atlas_uploads(")) {
    return 24;
  }

  return 0;
}
