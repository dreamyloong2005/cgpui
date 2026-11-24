#pragma once
#include "vulkan_loader_windows_internal.hpp"
#include "vulkan_missing_image_native_draw_sequence_test_support.hpp"
#include <cstddef>
#include <cstdint>

namespace cgpui_test {
struct VulkanNativeSubmissionObservation {
  bool reliable = true;
  unsigned begin_count = 0;
  unsigned successful_submit_count = 0;
  unsigned submitted_buffer_count = 0;
  VkCommandBuffer buffer = VK_NULL_HANDLE;
  std::uint64_t recording_epoch = 0;
  unsigned recorded_phase = 0;
  unsigned direct_draw_count = 0, indexed_draw_count = 0;
  bool reused = false;
  VulkanObservedDrawSequence sequence;
};
// Process-local forwarding observer: all callbacks call the real imported API.
// Only this serial test process may install one; no production state is exposed.
class VulkanNativeSubmissionTrace {
 public:
  VulkanNativeSubmissionTrace();
  ~VulkanNativeSubmissionTrace();
  VulkanNativeSubmissionTrace(const VulkanNativeSubmissionTrace&) = delete;
  VulkanNativeSubmissionTrace& operator=(const VulkanNativeSubmissionTrace&) = delete;
  void begin_frame(unsigned phase);
  [[nodiscard]] VulkanNativeSubmissionObservation observation() const;
 private:
  bool installed_ = false;
};
} // namespace cgpui_test
