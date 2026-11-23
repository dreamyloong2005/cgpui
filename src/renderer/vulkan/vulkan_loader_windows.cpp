#include "vulkan_loader_windows_internal.hpp"

#if defined(_WIN32)
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>

namespace cgpui {
Result<void> ensure_vulkan_windows_loader() {
  static std::mutex mutex;
  static bool ready = false;
  const std::lock_guard lock(mutex);
  if (ready) return {};

  // Safe default search: app directory, System32 and explicitly registered
  // user DLL directories, never the current directory or ambient PATH.
  std::unique_ptr<std::remove_pointer_t<HMODULE>, decltype(&FreeLibrary)> module(
      LoadLibraryExW(L"vulkan-1.dll", nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS), FreeLibrary);
  if (!module) return std::unexpected(Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = "Could not load Vulkan loader vulkan-1.dll (Win32 error " +
          std::to_string(GetLastError()) + ")"});

  auto candidate = resolve_vulkan_windows_functions(module.get(), GetProcAddress);
  if (!candidate) return std::unexpected(candidate.error());
  // No allocation or callback after validation. All factory admissions hold
  // this mutex, and later calls only read the successfully published pointers.
#define CGPUI_VULKAN_ENTRY(name) name = candidate->name;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
  // As with the former static import, keep code valid for retained resources
  // and late/static destructors. The OS releases this reference at process exit.
  (void)module.release();
  ready = true;
  return {};
}
} // namespace cgpui
#endif
