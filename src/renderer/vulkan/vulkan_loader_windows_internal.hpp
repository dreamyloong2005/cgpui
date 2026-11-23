#pragma once

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_win32.h>
#include "cgpui/renderer/renderer.hpp"

namespace cgpui {
// Private typed imports, shadowing the SDK's global prototypes only inside
// this backend. No public header or non-Windows Vulkan call path changes.
struct VulkanWindowsFunctions {
#define CGPUI_VULKAN_ENTRY(name) PFN_##name name = nullptr;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
};
using VulkanWindowsResolver = FARPROC(WINAPI*)(HMODULE, LPCSTR);

// Pure binding seam: resolve a complete candidate, never publish a partial one.
Result<VulkanWindowsFunctions> resolve_vulkan_windows_functions(
    HMODULE module, VulkanWindowsResolver resolver);

// Every real caller must first acquire this admission. Success publishes once
// and retains the module for process lifetime; a failed attempt is retryable.
Result<void> ensure_vulkan_windows_loader();
#define CGPUI_VULKAN_ENTRY(name) extern PFN_##name name;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
} // namespace cgpui
#endif
