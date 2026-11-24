#include "vulkan_missing_image_native_trace_test_support.hpp"
#include <array>

namespace cgpui_test {
namespace {
struct BufferRecord {
  VkCommandBuffer buffer = VK_NULL_HANDLE;
  std::uint64_t epoch = 0, begun_frame = 0;
  unsigned phase = 0, direct_draws = 0, submissions = 0, indexed_draws = 0;
  bool ended = false;
  VulkanObservedDrawSequence sequence;
};
struct TraceState {
  PFN_vkBeginCommandBuffer begin = nullptr;
  PFN_vkEndCommandBuffer end = nullptr;
  PFN_vkCmdDraw draw = nullptr;
  PFN_vkQueueSubmit submit = nullptr;
  PFN_vkCmdDrawIndexed indexed = nullptr;
  PFN_vkResetCommandBuffer reset = nullptr;
  PFN_vkFreeCommandBuffers free_buffers = nullptr;
  std::array<BufferRecord, 64> buffers{}; // No allocation/exception across Vulkan ABI.
  std::uint64_t frame = 0, epoch = 0;
  unsigned phase = 0;
  bool active = false, overflow = false;
  VulkanNativeSubmissionObservation observation;
} trace;
BufferRecord* find(VkCommandBuffer buffer, bool create, bool required = true) {
  for (auto& value : trace.buffers) if (value.buffer == buffer) return &value;
  if (create) for (auto& value : trace.buffers) if (!value.buffer) { value.buffer = buffer; return &value; }
  if (required) { trace.observation.reliable = false; trace.overflow = true; }
  return nullptr;
}
VKAPI_ATTR VkResult VKAPI_CALL begin(VkCommandBuffer buffer, const VkCommandBufferBeginInfo* info) {
  const VkResult result = trace.begin(buffer, info);
  if (result == VK_SUCCESS) {
    ++trace.observation.begin_count;
    if (auto* record = find(buffer, true)) *record = {buffer, ++trace.epoch, trace.frame, trace.phase};
  }
  return result; // Never invent success or replace real failure.
}
VKAPI_ATTR VkResult VKAPI_CALL end(VkCommandBuffer buffer) {
  const VkResult result = trace.end(buffer);
  if (auto* record = find(buffer, false)) record->ended = result == VK_SUCCESS;
  return result;
}
VKAPI_ATTR void VKAPI_CALL draw(VkCommandBuffer buffer, std::uint32_t vertices,
    std::uint32_t instances, std::uint32_t first, std::uint32_t first_instance) {
  trace.draw(buffer, vertices, instances, first, first_instance);
  if (auto* record = find(buffer, false)) {
    ++record->direct_draws;
    append_vulkan_observed_draw(record->sequence,
        {VulkanObservedDrawKind::direct, vertices, instances, first, 0, first_instance});
  }
}
VKAPI_ATTR void VKAPI_CALL indexed(VkCommandBuffer buffer, std::uint32_t count,
    std::uint32_t instances, std::uint32_t first, std::int32_t offset, std::uint32_t first_instance) {
  trace.indexed(buffer, count, instances, first, offset, first_instance);
  if (auto* record = find(buffer, false)) {
    ++record->indexed_draws;
    append_vulkan_observed_draw(record->sequence,
        {VulkanObservedDrawKind::indexed, count, instances, first, offset, first_instance});
  }
}
VKAPI_ATTR VkResult VKAPI_CALL reset(VkCommandBuffer buffer, VkCommandBufferResetFlags flags) {
  const VkResult result = trace.reset(buffer, flags);
  if (result == VK_SUCCESS) if (auto* record = find(buffer, false, false)) {
    record->ended = false; record->submissions = 0; record->sequence = {};
  }
  return result;
}
VKAPI_ATTR void VKAPI_CALL free_buffers(VkDevice device, VkCommandPool pool,
    std::uint32_t count, const VkCommandBuffer* buffers) {
  trace.free_buffers(device, pool, count, buffers);
  for (std::uint32_t i = 0; i < count; ++i)
    if (auto* record = find(buffers[i], false, false)) *record = {};
}
VKAPI_ATTR VkResult VKAPI_CALL submit(VkQueue queue, std::uint32_t count, const VkSubmitInfo* submits, VkFence fence) {
  const VkResult result = trace.submit(queue, count, submits, fence);
  if (result != VK_SUCCESS) return result;
  bool observed_packet = false;
  for (std::uint32_t i = 0; i < count; ++i) for (std::uint32_t j = 0; j < submits[i].commandBufferCount; ++j) {
    auto* record = find(submits[i].pCommandBuffers[j], false, false);
    // Identify this scene's buffer by recorded work, never a bare handle or packet position.
    if (record && record->indexed_draws == 2) {
      observed_packet = true;
      auto& o = trace.observation; ++o.submitted_buffer_count;
      o.reliable = o.reliable && record->ended;
      o.buffer = record->buffer; o.recording_epoch = record->epoch;
      o.recorded_phase = record->phase; o.direct_draw_count = record->direct_draws;
      o.indexed_draw_count = record->indexed_draws;
      o.sequence = record->sequence; // Actual recorded order survives native command-buffer reuse.
      o.reused = record->submissions != 0 && record->begun_frame != trace.frame && o.begin_count == 0;
      ++record->submissions;
    }
  }
  trace.observation.successful_submit_count += observed_packet;
  return result;
}
}
VulkanNativeSubmissionTrace::VulkanNativeSubmissionTrace() {
  if (trace.active || !cgpui::vkBeginCommandBuffer || !cgpui::vkEndCommandBuffer ||
      !cgpui::vkCmdDraw || !cgpui::vkQueueSubmit || !cgpui::vkCmdDrawIndexed ||
      !cgpui::vkResetCommandBuffer || !cgpui::vkFreeCommandBuffers) return;
  trace = {};
  trace.begin = cgpui::vkBeginCommandBuffer; trace.end = cgpui::vkEndCommandBuffer;
  trace.draw = cgpui::vkCmdDraw; trace.submit = cgpui::vkQueueSubmit;
  trace.indexed = cgpui::vkCmdDrawIndexed; trace.reset = cgpui::vkResetCommandBuffer;
  trace.free_buffers = cgpui::vkFreeCommandBuffers;
  cgpui::vkBeginCommandBuffer = begin; cgpui::vkEndCommandBuffer = end;
  cgpui::vkCmdDraw = draw; cgpui::vkQueueSubmit = submit;
  cgpui::vkCmdDrawIndexed = indexed; cgpui::vkResetCommandBuffer = reset;
  cgpui::vkFreeCommandBuffers = free_buffers;
  installed_ = trace.active = true;
}
VulkanNativeSubmissionTrace::~VulkanNativeSubmissionTrace() {
  if (!installed_) return;
  cgpui::vkBeginCommandBuffer = trace.begin; cgpui::vkEndCommandBuffer = trace.end;
  cgpui::vkCmdDraw = trace.draw; cgpui::vkQueueSubmit = trace.submit;
  cgpui::vkCmdDrawIndexed = trace.indexed; cgpui::vkResetCommandBuffer = trace.reset;
  cgpui::vkFreeCommandBuffers = trace.free_buffers;
  trace.active = false;
}
void VulkanNativeSubmissionTrace::begin_frame(unsigned phase) {
  ++trace.frame; trace.phase = phase; trace.observation = {};
  trace.observation.reliable = installed_ && !trace.overflow;
}
VulkanNativeSubmissionObservation VulkanNativeSubmissionTrace::observation() const {
  auto result = trace.observation; result.reliable = result.reliable && installed_; return result;
}
} // namespace cgpui_test
