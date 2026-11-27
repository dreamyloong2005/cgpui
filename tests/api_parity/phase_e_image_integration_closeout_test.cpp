#include <array>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = CGPUI_SOURCE_ROOT; root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }
  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

struct SourceEvidence {
  const char* path;
  const char* needle;
};

} // namespace

int main() {
  const std::string manifests = read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/renderer/mcpp.toml");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  if (manifests.empty() ||
      roadmap.empty() ||
      ledger_md.empty() ||
      ledger_json.empty()) {
    return 1;
  }

  if (!contains(manifests,
                "\"phase_e_image_integration_closeout_test.cpp\"") ||
      !contains(
          manifests,
          "\"phase_e_image_integration_closeout_test.cpp\"")) {
    return 2;
  }

  constexpr std::array target_evidence{
      "\"vulkan_image_texture_resources_test.cpp\"",
      "\"vulkan_image_texture_upload_test.cpp\"",
      "\"vulkan_image_texture_descriptor_test.cpp\"",
      "\"vulkan_image_draw_recording_test.cpp\"",
      "\"vulkan_image_tint_opacity_test.cpp\"",
      "\"vulkan_image_texture_cache_test.cpp\"",
      "\"vulkan_image_texture_invalidation_test.cpp\"",
  };
  for (std::size_t index = 0; index < target_evidence.size(); ++index) {
    if (!contains(manifests, target_evidence[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "tests/api_parity/phase_e_image_integration_closeout_test.cpp",
      "persistent image texture resources",
      "explicit bitmap upload transport",
      "nearest/linear sampling descriptors",
      "stable authored image interleaving",
      "multiplicative image tint",
      "frame-generation image cache",
      "deduplicated image invalidations"};
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
      !contains(ledger_md, required_docs[index]) ||
      !contains(ledger_json, required_docs[index])) {
      return 30 + static_cast<int>(index);
    }
  }

  constexpr std::array module_evidence{
      SourceEvidence{"include/cgpui/renderer/image_sampling.hpp",
                     "enum class ImageSamplingMode"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_image_texture_resources_internal.hpp",
          "struct VulkanImageTextureResource"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_image_texture_uploads_internal.hpp",
          "struct VulkanImageTextureStagingUpload"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_image_texture_descriptors_internal.hpp",
          "vulkan_image_texture_descriptor_capacity = 256"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_image_draw_recording_internal.hpp",
          "struct VulkanImageDrawCommand"},
      SourceEvidence{"src/renderer/vulkan/vulkan_image_color_internal.hpp",
                     "vulkan_resolve_image_tint("},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_image_texture_cache_internal.hpp",
          "vulkan_image_texture_cache_max_idle_frames = 120"},
      SourceEvidence{
          "src/renderer/vulkan/vulkan_image_texture_invalidation_internal.hpp",
          "vulkan_invalidate_image_texture_resources("},
  };
  for (std::size_t index = 0; index < module_evidence.size(); ++index) {
    const std::string source = read_source(module_evidence[index].path);
    if (source.empty() || !contains(source, module_evidence[index].needle)) {
      return 60 + static_cast<int>(index);
    }
  }

  constexpr std::array behavior_evidence{
      SourceEvidence{"tests/renderer/vulkan_image_texture_resources_test.cpp",
                     "test_rgba_texture_create_infos"},
      SourceEvidence{"tests/renderer/vulkan_image_texture_upload_test.cpp",
                     "test_explicit_frame_upload_transport"},
      SourceEvidence{
          "tests/renderer/vulkan_image_texture_descriptor_test.cpp",
          "test_sampler_and_descriptor_create_info"},
      SourceEvidence{"tests/renderer/vulkan_image_draw_recording_test.cpp",
                     "test_stable_order_resolves_image_draws"},
      SourceEvidence{"tests/renderer/vulkan_image_tint_opacity_test.cpp",
                     "test_authored_tint_multiplies_sample_and_opacity_once"},
      SourceEvidence{"tests/renderer/vulkan_image_texture_cache_test.cpp",
                     "test_idle_boundary_and_eviction"},
      SourceEvidence{
          "tests/renderer/vulkan_image_texture_invalidation_test.cpp",
          "test_resource_invalidation_is_idempotent"},
      SourceEvidence{"tests/renderer/vulkan_frame_lifetime_test.cpp",
                     "test_image_texture_invalidation_frame"},
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    const std::string source = read_source(behavior_evidence[index].path);
    if (source.empty() || !contains(source, behavior_evidence[index].needle)) {
      return 80 + static_cast<int>(index);
    }
  }

  return 0;
}
