#include "vulkan_backdrop_blur_recording_internal.hpp"

#include "vulkan_clip_scissor_internal.hpp"

namespace cgpui {
namespace {

void begin_pass(VkCommandBuffer command_buffer, VkRenderPass pass,
                VkFramebuffer framebuffer, VkRect2D area, Color clear) {
  VkClearValue clear_value{};
  clear_value.color = VkClearColorValue{{clear.r, clear.g, clear.b, clear.a}};
  const VkRenderPassBeginInfo info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = pass,
      .framebuffer = framebuffer,
      .renderArea = area,
      .clearValueCount = 1,
      .pClearValues = &clear_value,
  };
  vkCmdBeginRenderPass(command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
}

// Ends an offscreen pass and makes its target readable by later fragment
// shaders (and writable again by later passes).
void end_offscreen_pass(VkCommandBuffer command_buffer, VkImage image) {
  vkCmdEndRenderPass(command_buffer);
  const VkImageMemoryBarrier barrier{
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
      .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                           .levelCount = 1,
                           .layerCount = 1},
  };
  vkCmdPipelineBarrier(
      command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1,
      &barrier);
}

void set_area(VkCommandBuffer command_buffer, VkRect2D area) {
  const VkViewport viewport{
      .x = static_cast<float>(area.offset.x),
      .y = static_cast<float>(area.offset.y),
      .width = static_cast<float>(area.extent.width),
      .height = static_cast<float>(area.extent.height),
      .minDepth = 0.0F,
      .maxDepth = 1.0F,
  };
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);
  vkCmdSetScissor(command_buffer, 0, 1, &area);
}

void bind_effect(VkCommandBuffer command_buffer,
                 const VulkanBackdropBlurResources& resources,
                 VkPipeline pipeline, std::size_t source,
                 const VulkanBackdropBlurPushConstants& push) {
  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdBindDescriptorSets(
      command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
      resources.pipeline_layout, 0, 1,
      &resources.targets[source].descriptor_set, 0, nullptr);
  vkCmdPushConstants(
      command_buffer, resources.pipeline_layout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
      sizeof(push), &push);
}

// One fullscreen-triangle pass from `source` into `destination` texels.
void record_effect_pass(VkCommandBuffer command_buffer,
                        const VulkanBackdropBlurResources& resources,
                        VkPipeline pipeline, std::size_t source,
                        std::size_t destination, VkExtent2D texels,
                        const VulkanBackdropBlurPushConstants& push) {
  const VkRect2D area{.offset = {0, 0}, .extent = texels};
  begin_pass(command_buffer, resources.blur_pass,
             resources.targets[destination].framebuffer, area, Color{});
  set_area(command_buffer, area);
  bind_effect(command_buffer, resources, pipeline, source, push);
  vkCmdDraw(command_buffer, 3, 1, 0, 0);
  end_offscreen_pass(command_buffer, resources.targets[destination].image);
}

// Scene region -> ping (downsample), ping -> pong (x), pong -> ping (y).
void record_blur(VkCommandBuffer command_buffer,
                 const VulkanBackdropBlurResources& resources,
                 const VulkanBackdropBlurPass& pass) {
  const std::array<std::int32_t, 2> texel_max{
      static_cast<std::int32_t>(pass.texels.width) - 1,
      static_cast<std::int32_t>(pass.texels.height) - 1};
  const VulkanBackdropBlurPushConstants downsample{
      .source_origin = {pass.region.offset.x, pass.region.offset.y},
      .source_max = {
          pass.region.offset.x +
              static_cast<std::int32_t>(pass.region.extent.width) - 1,
          pass.region.offset.y +
              static_cast<std::int32_t>(pass.region.extent.height) - 1},
      .factor = static_cast<std::int32_t>(pass.factor),
  };
  record_effect_pass(command_buffer, resources, resources.downsample_pipeline,
                     kVulkanBackdropSceneTarget, kVulkanBackdropPingTarget,
                     pass.texels, downsample);
  VulkanBackdropBlurPushConstants blur{
      .source_max = texel_max,
      .direction = {1, 0},
      .radius = pass.radius,
      .sigma = pass.sigma,
  };
  record_effect_pass(command_buffer, resources, resources.blur_pipeline,
                     kVulkanBackdropPingTarget, kVulkanBackdropPongTarget,
                     pass.texels, blur);
  blur.direction = {0, 1};
  record_effect_pass(command_buffer, resources, resources.blur_pipeline,
                     kVulkanBackdropPongTarget, kVulkanBackdropPingTarget,
                     pass.texels, blur);
}

void record_composite(VkCommandBuffer command_buffer,
                      const VulkanBackdropBlurResources& resources,
                      const VulkanBackdropBlurPass& pass) {
  const VulkanClipScissorResolution clip = vulkan_resolve_clip_stack_scissor(
      resources.extent, pass.clip_rect, RendererClipStackRecord{});
  if (!clip.visible) return;
  set_area(command_buffer, VkRect2D{.offset = {0, 0}, .extent = resources.extent});
  vkCmdSetScissor(command_buffer, 0, 1, &clip.scissor);
  const VulkanBackdropBlurPushConstants push{
      .framebuffer_size = {static_cast<float>(resources.extent.width),
                           static_cast<float>(resources.extent.height)},
      .source_max = {static_cast<std::int32_t>(pass.texels.width) - 1,
                     static_cast<std::int32_t>(pass.texels.height) - 1},
  };
  bind_effect(command_buffer, resources, resources.composite_pipeline,
              kVulkanBackdropPingTarget, push);
  const VkBuffer buffer = resources.vertices.buffer;
  constexpr VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(command_buffer, 0, 1, &buffer, &offset);
  vkCmdDraw(command_buffer, kVulkanBackdropBlurVerticesPerPass, 1,
            pass.first_vertex, 0);
}

} // namespace

std::size_t vulkan_record_backdrop_blur_frame(
    VkCommandBuffer command_buffer,
    const VulkanBackdropBlurFrame& frame,
    const VulkanBackdropBlurPresentTarget& present,
    std::span<const VulkanFrameDrawOrderEntry> draw_order,
    const VulkanFrameDrawSegmentRecorder& record_segment) {
  const VulkanBackdropBlurResources& resources = *frame.resources;
  const VkRect2D full{.offset = {0, 0}, .extent = resources.extent};
  std::size_t offscreen_passes = 0;
  std::size_t segment_start = 0;
  std::size_t next_pass = 0;
  const VulkanBackdropBlurPass* pending = nullptr;
  for (std::size_t index = 0; index <= draw_order.size(); ++index) {
    const bool last = index == draw_order.size();
    if (!last) {
      const VulkanFrameDrawOrderEntry entry = draw_order[index];
      if (entry.primitive_kind != RendererPrimitiveKind::backdrop_blur) continue;
      while (next_pass < frame.passes.size() &&
             frame.passes[next_pass].source_index < entry.command_index) {
        ++next_pass;
      }
      if (next_pass == frame.passes.size() ||
          frame.passes[next_pass].source_index != entry.command_index) {
        continue;
      }
    }
    begin_pass(command_buffer,
               offscreen_passes == 0 ? resources.scene_clear_pass
                                     : resources.scene_load_pass,
               resources.targets[kVulkanBackdropSceneTarget].framebuffer, full,
               present.clear_color);
    if (pending != nullptr) record_composite(command_buffer, resources, *pending);
    record_segment(draw_order.subspan(segment_start, index - segment_start));
    end_offscreen_pass(command_buffer,
                       resources.targets[kVulkanBackdropSceneTarget].image);
    ++offscreen_passes;
    if (last) break;
    pending = &frame.passes[next_pass++];
    record_blur(command_buffer, resources, *pending);
    offscreen_passes += 3;
    segment_start = index + 1;
  }
  begin_pass(command_buffer, present.render_pass, present.framebuffer,
             VkRect2D{.offset = {0, 0}, .extent = present.extent},
             present.clear_color);
  set_area(command_buffer, VkRect2D{.offset = {0, 0}, .extent = present.extent});
  bind_effect(command_buffer, resources, resources.present_pipeline,
              kVulkanBackdropSceneTarget, VulkanBackdropBlurPushConstants{});
  vkCmdDraw(command_buffer, 3, 1, 0, 0);
  vkCmdEndRenderPass(command_buffer);
  return offscreen_passes;
}

} // namespace cgpui
