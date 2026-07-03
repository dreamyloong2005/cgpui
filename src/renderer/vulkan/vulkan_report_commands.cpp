#include "vulkan_report_internal.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

namespace cgpui {
namespace {

void append_command_batch(
    std::vector<RendererCommandBatch>& batches,
    RendererCommandBatchKey key,
    std::size_t command_index) {
  if (batches.empty() ||
      !vulkan_same_command_batch_key(batches.back().key, key)) {
    batches.push_back(RendererCommandBatch{.key = std::move(key)});
  }

  RendererCommandBatch& batch = batches.back();
  batch.command_count += 1;
  batch.command_indices.push_back(command_index);
}

} // namespace

bool vulkan_is_supported_renderer_primitive(
    RendererPrimitiveKind primitive_kind) {
  return primitive_kind == RendererPrimitiveKind::solid_rect ||
         primitive_kind == RendererPrimitiveKind::rounded_rect ||
         primitive_kind == RendererPrimitiveKind::text ||
         primitive_kind == RendererPrimitiveKind::text_selection ||
         primitive_kind == RendererPrimitiveKind::text_caret ||
         primitive_kind == RendererPrimitiveKind::image;
}

RendererUnsupportedCommandDiagnostic
vulkan_make_unsupported_renderer_command_diagnostic(
    const RendererCommandStreamItem& command) {
  std::string message = "Vulkan renderer does not support ";
  message += renderer_primitive_kind_name(command.primitive_kind);
  message += " commands yet";
  return RendererUnsupportedCommandDiagnostic{
      .primitive_kind = command.primitive_kind,
      .command_index = command.command_index,
      .reason = RendererUnsupportedCommandReason::unsupported_primitive,
      .message = std::move(message),
  };
}

std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets) {
  const std::vector<RendererCommandStreamItem> commands =
      vulkan_build_renderer_command_stream(
          rects,
          rounded_rects,
          text_draws,
          selections,
          carets,
          std::span<const ImageDraw>{});
  return vulkan_build_renderer_command_report(commands).batches;
}

std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws) {
  return vulkan_build_renderer_command_batches(
      rects,
      rounded_rects,
      text_draws,
      std::span<const TextSelectionDraw>{},
      std::span<const TextCaretDraw>{});
}

std::vector<RendererCommandBatch> vulkan_build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws) {
  return vulkan_build_renderer_command_batches(
      rects,
      std::span<const RoundedRectDraw>{},
      text_draws);
}

RendererCommandReport vulkan_build_renderer_command_report(
    std::span<const RendererCommandStreamItem> commands) {
  RendererCommandReport report;
  report.batches.reserve(commands.size());

  for (const RendererCommandStreamItem& command : commands) {
    if (!vulkan_is_supported_renderer_primitive(command.primitive_kind)) {
      report.unsupported_commands.push_back(
          vulkan_make_unsupported_renderer_command_diagnostic(command));
      report.unsupported_command_count += 1;
      continue;
    }

    append_command_batch(
        report.batches,
        RendererCommandBatchKey{
            .primitive_kind = command.primitive_kind,
            .clip_rect = command.clip_rect,
            .clip_stack = command.clip_stack,
            .composition_stack = command.composition_stack,
            .metadata = command.metadata,
        },
        command.command_index);
    report.supported_command_count += 1;
    if (!command.clip_stack.empty()) {
      report.clip_stack_record_count += 1;
      report.max_clip_stack_depth = std::max(
          report.max_clip_stack_depth,
          command.clip_stack.full_depth);
    }
    if (!command.composition_stack.empty()) {
      report.composition_stack_record_count += 1;
      report.max_composition_stack_depth = std::max(
          report.max_composition_stack_depth,
          command.composition_stack.full_depth);
    }
  }

  report.submission_plan_records = vulkan_build_renderer_submission_plan(
      report.batches,
      std::span<const std::vector<VulkanTextDrawAtlasPageUsage>>{},
      report.text_render.text_sampler_pipeline);
  vulkan_record_submission_plan_statistics(report);
  return report;
}

} // namespace cgpui
