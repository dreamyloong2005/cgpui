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
      "src/renderer/vulkan/vulkan_glyph_atlas_uploads_internal.hpp",
      "src/renderer/vulkan/vulkan_glyph_atlas_resources_internal.hpp",
      "src/renderer/vulkan/vulkan_command_recording.cpp",
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
      !contains(renderer_header, "#include \"cgpui/renderer/glyph_atlas.hpp\"") ||
      !contains(renderer_header,
                "#include \"cgpui/renderer/renderer_frame.hpp\"")) {
    return 7;
  }
  if (line_count(renderer_header) > 40 ||
      contains(renderer_header, "class Renderer") ||
      contains(renderer_header, "struct RendererCommandReport") ||
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
  const std::string glyph_atlas =
      read_source("include/cgpui/renderer/glyph_atlas.hpp");
  const std::string glyph_atlas_types =
      read_source("include/cgpui/renderer/glyph_atlas_types.hpp");
  const std::string glyph_uploads =
      read_source("include/cgpui/renderer/glyph_uploads.hpp");
  const std::string image_uploads =
      read_source("include/cgpui/renderer/image_uploads.hpp");
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
      line_count(renderer_reports) > 40 ||
      contains(renderer_reports, "struct RendererCommandReport") ||
      contains(renderer_reports, "struct RendererFrameReport")) {
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
      !contains(glyph_atlas_types, "struct GlyphAtlasEntry") ||
      !contains(glyph_uploads, "struct GlyphAtlasUploadBatch") ||
      !contains(image_uploads, "struct ImageUploadBatch") ||
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
      line_count(glyph_texture_resources) > 220 ||
      line_count(glyph_atlas_production) > 220 ||
      line_count(glyph_cache) > 220 ||
      line_count(renderer_text_reports) > 220 ||
      line_count(renderer_image_reports) > 220 ||
      line_count(renderer_submission_reports) > 220 ||
      line_count(renderer_geometry_reports) > 220 ||
      line_count(renderer_frame_reports) > 220) {
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
      !contains(surface_selection, "choose_vulkan_present_mode(") ||
      !contains(surface_selection, "choose_vulkan_composite_alpha(") ||
      !contains(surface_selection, "choose_vulkan_extent(") ||
      contains(surface_selection, "require_vk_success(")) {
    return 35;
  }

  const std::string command_recording =
      read_source("src/renderer/vulkan/vulkan_command_recording.cpp");
  if (line_count(command_recording) > 180 ||
      !contains(command_recording, "make_clear_rect(") ||
      !contains(command_recording, "record_vulkan_frame_command_buffer(") ||
      !contains(command_recording, "vkCmdBeginRenderPass") ||
      !contains(command_recording, "vkCmdClearAttachments") ||
      contains(command_recording, "choose_vulkan_surface_format(")) {
    return 36;
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
  if (presentation_recovery.empty()) {
    return 47;
  }
  if (line_count(presentation) > 150 ||
      !contains(presentation, "VulkanRendererState::present_frame(") ||
      !contains(presentation, "vkAcquireNextImageKHR") ||
      !contains(presentation, "vkQueueSubmit") ||
      !contains(presentation, "vkQueuePresentKHR") ||
      contains(presentation,
               "Result<void> VulkanRendererState::recover_after_failed_submit(") ||
      contains(presentation,
               "Result<void> VulkanRendererState::recover_after_failed_acquire(") ||
      contains(presentation,
               "Result<void> VulkanRendererState::recover_after_failed_present(") ||
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
  if (line_count(glyph_atlas_uploads_internal) > 80 ||
      line_count(command_recording_internal) > 40 ||
      line_count(device_memory) > 50 ||
      line_count(glyph_atlas_staging) > 230 ||
      line_count(glyph_atlas_upload_recording) > 170 ||
      line_count(glyph_atlas_frame) > 80 ||
      !contains(glyph_atlas_staging, "VK_BUFFER_USAGE_TRANSFER_SRC_BIT") ||
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
  if (line_count(text_vertex_buffer_header) > 60 ||
      line_count(text_vertex_buffer) > 190 ||
      !contains(text_vertex_buffer_header,
                "struct VulkanTextVertexBufferResources") ||
      !contains(text_vertex_buffer, "vulkan_build_text_vertices(") ||
      !contains(text_vertex_buffer, "VK_BUFFER_USAGE_VERTEX_BUFFER_BIT") ||
      !contains(text_vertex_buffer, "vkMapMemory") ||
      contains(command_recording, "vulkan_build_text_vertices(")) {
    return 64;
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
      !contains(command_recording, "vulkan_record_text_draws(") ||
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

  const std::string report_image_uploads =
      read_source("src/renderer/vulkan/vulkan_report_image_uploads.cpp");
  if (line_count(report_image_uploads) > 80 ||
      !contains(report_image_uploads, "vulkan_plan_image_uploads(") ||
      !contains(report_image_uploads, "ImageUploadRegion") ||
      contains(report_image_uploads, "vulkan_plan_glyph_atlas_uploads(") ||
      contains(report_image_uploads, "vulkan_build_textured_glyph_quads(")) {
    return 41;
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
      !contains(report_geometry, "vulkan_build_text_selection_geometry(") ||
      !contains(report_geometry, "vulkan_build_text_caret_geometry(") ||
      contains(report_geometry, "vulkan_plan_glyph_atlas_uploads(")) {
    return 24;
  }

  return 0;
}
