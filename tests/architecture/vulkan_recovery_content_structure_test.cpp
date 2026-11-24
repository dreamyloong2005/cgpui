#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error("missing Vulkan recovery content owner: " + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, const char* token) { return source.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void cap(const std::string& path, unsigned limit) {
  const auto source = read(path);
  require(std::count(source.begin(), source.end(), '\n') <= limit, "recovery content module exceeds its cap");
}
}
int main() {
  try {
    const auto frame = read("src/renderer/vulkan/vulkan_image_texture_frame.cpp");
    const auto retention = read("src/renderer/vulkan/vulkan_image_retention.cpp");
    require(has(retention, "[&](const VulkanRetainedImage& entry)") &&
        has(retention, "for (auto& entry : retained)"),
        "pruning uses a read-only removal predicate and updates surviving ages separately");
    const auto validation = frame.find("vulkan_validate_retained_image_requests(");
    const auto invalidation = frame.find("vulkan_invalidate_retained_images(");
    require(validation != std::string::npos && invalidation != std::string::npos && validation < invalidation &&
        has(read("src/renderer/vulkan/vulkan_image_retention_validation.cpp"), "renderer_same_image_allocation("),
        "recovery must preserve accepted allocation identity before any image mutation");
    const auto restore = read("src/renderer/vulkan/vulkan_image_restore.cpp");
    require(has(restore, "properties.limits.maxImageDimension2D") &&
        restore.find("properties.limits.maxImageDimension2D") < restore.find("vulkan_ensure_image_texture_resource(") &&
        has(restore, "entry.last_used_frame") && has(restore, "vulkan_image_texture_resource_matches("),
        "restoration preflights current device limits, preserves idle age and refuses evicted/mismatched uploads");
    const auto images = read("src/renderer/vulkan/vulkan_glyph_atlas_images.cpp");
    const auto glyphs = read("src/renderer/vulkan/vulkan_glyph_atlas_resources.cpp");
    require(has(images, "properties.limits.maxImageDimension2D") &&
        images.find("vulkan_glyph_atlas_extent(") < images.find("vkCreateImage(") &&
        glyphs.find("vulkan_validate_glyph_atlas_resources(") < glyphs.find("for (auto page ="),
        "glyph restoration checks replacement limits before integer conversion or native mutation");
    const auto glyph_validation = read("src/renderer/vulkan/vulkan_glyph_atlas_validation.cpp");
    require(has(glyph_validation, "std::isfinite(width)") && has(glyph_validation, "width > maximum_dimension") &&
        glyph_validation.find("width > maximum_dimension") < glyph_validation.find("static_cast<std::uint32_t>(width)"),
        "glyph extent overflow and nonfinite values must be rejected before conversion");
    const auto renderer = read("src/renderer/vulkan/vulkan_renderer.cpp");
    const auto present = renderer.find("Result<void> VulkanFrame::present()");
    require(present != std::string::npos &&
        renderer.find("generation_ !=", present) < renderer.find("if (finished_)", present) &&
        renderer.find("finished_ = true", present) < renderer.find("state_->present_frame(", present),
        "retired frames request retry; healthy finished frames cannot replay transferred image uploads");
    const auto registration = read("tests/renderer/mcpp.toml");
    for (const auto* name : {"retention", "accounting", "restore_policy", "retained_validation"}) {
      const auto file = std::string("vulkan_device_recovery_") + name + "_test.cpp";
      require(registration.find(file) != std::string::npos, "CPU recovery content regression is not registered");
      cap("tests/renderer/" + file, 100);
    }
    require(has(registration, "vulkan_glyph_atlas_extent_limit_test.cpp"), "glyph replacement-limit test is not registered");
    cap("tests/renderer/vulkan_glyph_atlas_extent_limit_test.cpp", 40);
    for (const auto* name : {"vulkan_acquire_sync_failure", "vulkan_image_create_failure", "vulkan_device_recovery_image_limits"}) {
      const auto leaf = std::string(name) + "_test_support";
      require(registration.find(leaf + ".cpp") != std::string::npos, "native failure support is not registered");
      cap("tests/renderer/" + leaf + ".cpp", 100);
      cap("tests/renderer/" + leaf + ".hpp", 35);
    }
    cap("src/renderer/vulkan/vulkan_image_retention_internal.hpp", 45);
    cap("src/renderer/vulkan/vulkan_image_retention.cpp", 80);
    cap("src/renderer/vulkan/vulkan_image_retention_validation.cpp", 40);
    cap("src/renderer/vulkan/vulkan_image_restore.cpp", 80);
    cap("src/renderer/vulkan/vulkan_glyph_atlas_validation_internal.hpp", 20);
    cap("src/renderer/vulkan/vulkan_glyph_atlas_validation.cpp", 50);
    cap("tests/architecture/vulkan_recovery_content_structure_test.cpp", 90);
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
