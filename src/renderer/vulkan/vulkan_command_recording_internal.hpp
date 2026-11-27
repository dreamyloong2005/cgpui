#pragma once

#include "vulkan_frame_command_reuse_internal.hpp"
#include "vulkan_frame_recording_input_internal.hpp"

namespace cgpui {

Result<VulkanFrameCommandRecordingResult> record_vulkan_frame_command_buffer(
    const VulkanFrameRecordingInput& input,
    VulkanFrameCommandReuseState& command_reuse_state);

} // namespace cgpui
