#include "metal_renderer_internal.hpp"

namespace cgpui {
namespace {

RendererCommandStreamItem stream_item(
    const MetalFrameCommands& commands,
    MetalFrameDrawOrderEntry entry) {
  RendererCommandStreamItem item{
      .primitive_kind = entry.primitive_kind,
      .command_index = entry.command_index};
  const auto assign = [&](const auto& draw) {
    item.clip_rect = draw.clip_rect;
    item.clip_stack = draw.clip_stack;
    item.composition_stack = draw.composition_stack;
    item.metadata = draw.metadata;
  };
  switch (entry.primitive_kind) {
    case RendererPrimitiveKind::solid_rect:
      assign(commands.solid_rects[entry.command_index]);
      break;
    case RendererPrimitiveKind::rounded_rect:
      assign(commands.rounded_rects[entry.command_index]);
      break;
    case RendererPrimitiveKind::text:
      assign(commands.text_draws[entry.command_index]);
      break;
    case RendererPrimitiveKind::text_selection:
      assign(commands.selections[entry.command_index]);
      break;
    case RendererPrimitiveKind::text_caret:
      assign(commands.carets[entry.command_index]);
      break;
    case RendererPrimitiveKind::image:
      assign(commands.image_draws[entry.command_index]);
      break;
  }
  return item;
}

bool same_batch(
    const RendererCommandBatch& batch,
    const RendererCommandStreamItem& item) {
  const auto& key = batch.key;
  return key.primitive_kind == item.primitive_kind &&
         key.clip_rect.has_value() == item.clip_rect.has_value() &&
         (!key.clip_rect ||
          (key.clip_rect->origin.x == item.clip_rect->origin.x &&
           key.clip_rect->origin.y == item.clip_rect->origin.y &&
           key.clip_rect->size.width == item.clip_rect->size.width &&
           key.clip_rect->size.height == item.clip_rect->size.height)) &&
         key.clip_stack == item.clip_stack &&
         key.composition_stack == item.composition_stack &&
         key.metadata == item.metadata;
}

}  // namespace

std::vector<RendererCommandBatch> metal_build_command_batches(
    const MetalFrameCommands& commands) {
  std::vector<RendererCommandBatch> batches;
  for (const MetalFrameDrawOrderEntry entry : commands.draw_order) {
    const RendererCommandStreamItem item = stream_item(commands, entry);
    if (batches.empty() || !same_batch(batches.back(), item)) {
      batches.push_back(RendererCommandBatch{
          .key = {
              .primitive_kind = item.primitive_kind,
              .clip_rect = item.clip_rect,
              .clip_stack = item.clip_stack,
              .composition_stack = item.composition_stack,
              .metadata = item.metadata}});
    }
    ++batches.back().command_count;
    batches.back().command_indices.push_back(item.command_index);
  }
  return batches;
}

}  // namespace cgpui
