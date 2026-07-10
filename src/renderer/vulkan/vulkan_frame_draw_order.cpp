#include "vulkan_frame_draw_order_internal.hpp"

namespace cgpui {

VulkanFrameDrawOrderCursor::VulkanFrameDrawOrderCursor(
    std::span<const VulkanFrameDrawOrderEntry> order,
    std::span<const VulkanRoundedRectDrawRange> solid_draws,
    std::span<const VulkanRoundedRectDrawRange> rounded_draws,
    std::span<const VulkanTextDrawCommand> text_commands,
    std::span<const VulkanImageDrawCommand> image_commands)
    : order_(order),
      solid_draws_(solid_draws),
      rounded_draws_(rounded_draws),
      text_commands_(text_commands),
      image_commands_(image_commands) {}

std::optional<VulkanResolvedFrameDraw> VulkanFrameDrawOrderCursor::next() {
  while (order_index_ < order_.size()) {
    const VulkanFrameDrawOrderEntry entry = order_[order_index_];
    if (entry.primitive_kind == RendererPrimitiveKind::solid_rect) {
      while (solid_index_ < solid_draws_.size() &&
             solid_draws_[solid_index_].source_index < entry.command_index) {
        ++solid_index_;
      }
      ++order_index_;
      if (solid_index_ < solid_draws_.size() &&
          solid_draws_[solid_index_].source_index == entry.command_index) {
        return VulkanResolvedFrameDraw{
            .resource_kind = VulkanFrameDrawResourceKind::solid_rect,
            .resource_index = solid_index_++,
        };
      }
      continue;
    }
    if (entry.primitive_kind == RendererPrimitiveKind::rounded_rect) {
      while (rounded_index_ < rounded_draws_.size() &&
             rounded_draws_[rounded_index_].source_index < entry.command_index) {
        ++rounded_index_;
      }
      ++order_index_;
      if (rounded_index_ < rounded_draws_.size() &&
          rounded_draws_[rounded_index_].source_index == entry.command_index) {
        return VulkanResolvedFrameDraw{
            .resource_kind = VulkanFrameDrawResourceKind::rounded_rect,
            .resource_index = rounded_index_++,
        };
      }
      continue;
    }
    if (entry.primitive_kind == RendererPrimitiveKind::image) {
      while (image_index_ < image_commands_.size() &&
             image_commands_[image_index_].image_draw_index <
                 entry.command_index) {
        ++image_index_;
      }
      ++order_index_;
      if (image_index_ < image_commands_.size() &&
          image_commands_[image_index_].image_draw_index ==
              entry.command_index) {
        return VulkanResolvedFrameDraw{
            .resource_kind = VulkanFrameDrawResourceKind::image,
            .resource_index = image_index_++,
        };
      }
      continue;
    }
    if (entry.primitive_kind == RendererPrimitiveKind::text) {
      while (text_index_ < text_commands_.size() &&
             text_commands_[text_index_].text_draw_index < entry.command_index) {
        ++text_index_;
      }
      if (text_index_ < text_commands_.size() &&
          text_commands_[text_index_].text_draw_index == entry.command_index) {
        const std::size_t resource_index = text_index_++;
        if (text_index_ == text_commands_.size() ||
            text_commands_[text_index_].text_draw_index != entry.command_index) {
          ++order_index_;
        }
        return VulkanResolvedFrameDraw{
            .resource_kind = VulkanFrameDrawResourceKind::text,
            .resource_index = resource_index,
        };
      }
    }
    ++order_index_;
  }
  return std::nullopt;
}

} // namespace cgpui
