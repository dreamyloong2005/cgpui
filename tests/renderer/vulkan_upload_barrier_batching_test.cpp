#include "vulkan_upload_barrier_batch_internal.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <type_traits>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
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

std::size_t occurrence_count(const std::string& text, const char* value) {
  std::size_t count = 0;
  std::size_t position = 0;
  while ((position = text.find(value, position)) != std::string::npos) {
    ++count;
    position += std::char_traits<char>::length(value);
  }
  return count;
}

template <typename Handle>
Handle fake_handle(std::uintptr_t value) {
  if constexpr (std::is_pointer_v<Handle>) {
    return reinterpret_cast<Handle>(value);
  } else {
    return static_cast<Handle>(value);
  }
}

int test_unique_targets_share_one_ordered_wave() {
  const std::array<cgpui::VulkanUploadImageBarrierRequest, 2> requests{
      cgpui::VulkanUploadImageBarrierRequest{
          .image = fake_handle<VkImage>(1),
          .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
      },
      cgpui::VulkanUploadImageBarrierRequest{
          .image = fake_handle<VkImage>(2),
          .old_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      },
  };
  const cgpui::VulkanUploadBarrierPlan plan =
      cgpui::vulkan_plan_upload_image_barriers(requests);
  if (plan.waves.size() != 1 ||
      plan.waves[0].upload_indices != std::vector<std::size_t>{0, 1}) {
    return 10;
  }
  const auto& transfer = plan.waves[0].to_transfer;
  const auto& readable = plan.waves[0].to_readable;
  if (transfer.barriers.size() != 2 || readable.barriers.size() != 2 ||
      transfer.destination_stage_mask != VK_PIPELINE_STAGE_TRANSFER_BIT ||
      (transfer.source_stage_mask & VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT) == 0 ||
      (transfer.source_stage_mask & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT) == 0 ||
      transfer.barriers[0].oldLayout != VK_IMAGE_LAYOUT_UNDEFINED ||
      transfer.barriers[1].oldLayout !=
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    return 11;
  }
  return readable.source_stage_mask == VK_PIPELINE_STAGE_TRANSFER_BIT &&
                 readable.destination_stage_mask ==
                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT &&
                 readable.barriers[0].oldLayout ==
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 readable.barriers[0].newLayout ==
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
             ? 0
             : 12;
}

int test_duplicate_target_starts_new_readable_based_wave() {
  const VkImage repeated = fake_handle<VkImage>(3);
  const std::array<cgpui::VulkanUploadImageBarrierRequest, 3> requests{
      cgpui::VulkanUploadImageBarrierRequest{
          .image = repeated,
          .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
      },
      cgpui::VulkanUploadImageBarrierRequest{
          .image = fake_handle<VkImage>(4),
          .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
      },
      cgpui::VulkanUploadImageBarrierRequest{
          .image = repeated,
          .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
      },
  };
  const cgpui::VulkanUploadBarrierPlan plan =
      cgpui::vulkan_plan_upload_image_barriers(requests);
  if (plan.waves.size() != 2 ||
      plan.waves[0].upload_indices != std::vector<std::size_t>{0, 1} ||
      plan.waves[1].upload_indices != std::vector<std::size_t>{2}) {
    return 20;
  }
  const auto& repeated_transfer = plan.waves[1].to_transfer.barriers[0];
  return repeated_transfer.image == repeated &&
                 repeated_transfer.oldLayout ==
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
                 repeated_transfer.srcAccessMask == VK_ACCESS_SHADER_READ_BIT
             ? 0
             : 21;
}

int test_empty_plan_has_no_barrier_wave() {
  const std::array<cgpui::VulkanUploadImageBarrierRequest, 0> requests{};
  return cgpui::vulkan_plan_upload_image_barriers(requests).waves.empty()
             ? 0
             : 30;
}

int test_structure_and_documentation() {
  const std::string header = read_source(
      "src/renderer/vulkan/vulkan_upload_barrier_batch_internal.hpp");
  const std::string source = read_source(
      "src/renderer/vulkan/vulkan_upload_barrier_batch.cpp");
  const std::string glyph = read_source(
      "src/renderer/vulkan/vulkan_glyph_atlas_upload_recording.cpp");
  const std::string image = read_source(
      "src/renderer/vulkan/vulkan_image_texture_upload_recording.cpp");
  const std::string structure = read_source(
      "tests/architecture/renderer_source_structure_test.cpp");
  if (header.empty() || source.empty() || glyph.empty() || image.empty() ||
      structure.empty()) {
    return 40;
  }
  if (!contains(header, "struct VulkanUploadBarrierPlan") ||
      !contains(source, "vulkan_plan_upload_image_barriers(") ||
      occurrence_count(source, "vkCmdPipelineBarrier") != 1 ||
      !contains(glyph, "vulkan_plan_upload_image_barriers(") ||
      !contains(image, "vulkan_plan_upload_image_barriers(") ||
      contains(glyph, "vkCmdPipelineBarrier") ||
      contains(image, "vkCmdPipelineBarrier") ||
      !contains(structure, "vulkan_upload_barrier_batch.cpp")) {
    return 41;
  }

  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 510",
      "ordered upload barrier waves",
      "batched transfer and shader-read transitions",
      "duplicate image targets start a new wave",
      "Step 511 swapchain recovery",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 50;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_unique_targets_share_one_ordered_wave();
      result != 0) {
    return result;
  }
  if (const int result = test_duplicate_target_starts_new_readable_based_wave();
      result != 0) {
    return result;
  }
  if (const int result = test_empty_plan_has_no_barrier_wave(); result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
