#include "vulkan_loader_windows_internal.hpp"

#if defined(_WIN32)
namespace cgpui {
#define CGPUI_VULKAN_ENTRY(name) PFN_##name name = nullptr;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY

Result<VulkanWindowsFunctions> resolve_vulkan_windows_functions(
    HMODULE module, VulkanWindowsResolver resolver) {
  if (!module || !resolver) return std::unexpected(Error{
      .code = ErrorCode::invalid_argument,
      .message = "Vulkan loader binding requires a module and resolver"});
  VulkanWindowsFunctions candidate;
#define CGPUI_VULKAN_ENTRY(name) \
  candidate.name = reinterpret_cast<PFN_##name>(resolver(module, #name)); \
  if (!candidate.name) return std::unexpected(Error{ \
      .code = ErrorCode::renderer_initialization_failed, \
      .message = "Vulkan loader vulkan-1.dll is missing required export " #name});
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
  return candidate;
}
} // namespace cgpui
#endif
