#include "vulkan_frame_diagnostic_snapshot_internal.hpp"

#include <array>
#include <ranges>

namespace cgpui {
namespace {

void append_resource(
    std::vector<RendererFrameResource>& resources,
    RendererDrawCounts& counts,
    RendererPrimitiveKind primitive_kind,
    std::size_t command_index,
    std::size_t resource_index) {
  resources.push_back(RendererFrameResource{
      .primitive_kind = primitive_kind,
      .command_index = command_index,
      .resource_index = resource_index,
  });
  renderer_add_draw_count(counts, primitive_kind);
}

bool has_draw_range(
    std::span<const VulkanRoundedRectDrawRange> draws,
    std::size_t command_index) {
  return std::ranges::any_of(draws, [&](const auto& draw) {
    return draw.source_index == command_index;
  });
}

bool submitted_resource_exists(
    std::span<const RendererFrameResource> submitted,
    RendererPrimitiveKind primitive_kind,
    std::size_t command_index) {
  return std::ranges::any_of(submitted, [&](const auto& resource) {
    return resource.primitive_kind == primitive_kind &&
           resource.command_index == command_index;
  });
}

} // namespace

VulkanFrameDiagnosticResources vulkan_build_frame_diagnostic_resources(
    std::span<const VulkanFrameDrawOrderEntry> draw_order,
    std::span<const VulkanRoundedRectDrawRange> solid_draws,
    std::span<const VulkanRoundedRectDrawRange> rounded_draws,
    std::span<const VulkanGlyphAtlasDrawBinding> text_bindings,
    std::span<const ImageDraw> image_draws,
    std::span<const RendererCommandBatch> command_batches) {
  VulkanFrameDiagnosticResources result;
  result.planned_resources.reserve(draw_order.size());
  result.submitted_resources.reserve(draw_order.size());
  for (const VulkanFrameDrawOrderEntry entry : draw_order) {
    if (entry.primitive_kind == RendererPrimitiveKind::text) {
      for (std::size_t index = 0; index < text_bindings.size(); ++index) {
        if (text_bindings[index].text_draw_index == entry.command_index) {
          append_resource(
              result.planned_resources,
              result.planned_draws,
              entry.primitive_kind,
              entry.command_index,
              index);
          append_resource(
              result.submitted_resources,
              result.submitted_draws,
              entry.primitive_kind,
              entry.command_index,
              index);
        }
      }
      continue;
    }

    append_resource(
        result.planned_resources,
        result.planned_draws,
        entry.primitive_kind,
        entry.command_index,
        entry.command_index);
    const bool submitted =
        (entry.primitive_kind == RendererPrimitiveKind::solid_rect &&
         has_draw_range(solid_draws, entry.command_index)) ||
        (entry.primitive_kind == RendererPrimitiveKind::rounded_rect &&
         has_draw_range(rounded_draws, entry.command_index)) ||
        (entry.primitive_kind == RendererPrimitiveKind::image &&
         entry.command_index < image_draws.size());
    if (submitted) {
      append_resource(
          result.submitted_resources,
          result.submitted_draws,
          entry.primitive_kind,
          entry.command_index,
          entry.command_index);
    }
  }

  const std::array supported{
      RendererPrimitiveKind::solid_rect,
      RendererPrimitiveKind::rounded_rect,
      RendererPrimitiveKind::text,
      RendererPrimitiveKind::image,
  };
  result.dropped_resources = classify_renderer_dropped_resources(
      result.planned_resources, result.submitted_resources, supported);
  for (const VulkanFrameDrawOrderEntry entry : draw_order) {
    if (submitted_resource_exists(
            result.submitted_resources,
            entry.primitive_kind,
            entry.command_index)) {
      ++result.submitted_command_count;
    }
  }
  for (const RendererCommandBatch& batch : command_batches) {
    const bool submitted = std::ranges::any_of(
        batch.command_indices,
        [&](std::size_t command_index) {
          return submitted_resource_exists(
              result.submitted_resources,
              batch.key.primitive_kind,
              command_index);
        });
    result.submitted_batch_count += submitted ? 1 : 0;
  }
  return result;
}

} // namespace cgpui
