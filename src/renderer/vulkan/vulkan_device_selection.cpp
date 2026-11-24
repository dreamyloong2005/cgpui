#include "vulkan_state_internal.hpp"
#include "vulkan_device_selection_internal.hpp"
#include <cstdlib>

namespace cgpui {
Result<void> VulkanRendererState::initialize_device_selection() {
  // Snapshot once before native creation; recovery never re-reads process state.
  const char* setting = std::getenv("CGPUI_VULKAN_DEVICE");
  auto selection = vulkan_parse_device_selection(setting ? std::string_view(setting) : std::string_view{});
  if (!selection) return std::unexpected(selection.error());
  requested_device_index_ = *selection;
  return {};
}
} // namespace cgpui
