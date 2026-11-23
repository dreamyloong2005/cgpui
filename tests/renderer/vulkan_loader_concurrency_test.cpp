#include "vulkan_loader_windows_internal.hpp"

#include <array>
#include <barrier>
#include <cstddef>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

namespace {
constexpr std::size_t thread_count = 16;
constexpr std::size_t required_count = 0
#define CGPUI_VULKAN_ENTRY(name) + 1
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
;

cgpui::VulkanWindowsFunctions published_functions() {
  cgpui::VulkanWindowsFunctions table;
#define CGPUI_VULKAN_ENTRY(name) table.name = cgpui::name;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
  return table;
}

bool same(const cgpui::VulkanWindowsFunctions& left,
          const cgpui::VulkanWindowsFunctions& right) {
#define CGPUI_VULKAN_ENTRY(name) if (left.name != right.name) return false;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
  return true;
}

bool complete(const cgpui::VulkanWindowsFunctions& table, HMODULE module) {
  if (!module) return false;
#define CGPUI_VULKAN_ENTRY(name) \
  if (!table.name || reinterpret_cast<FARPROC>(table.name) != GetProcAddress(module, #name)) return false;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
  return true;
}

struct Admission {
  bool success = false;
  std::string error;
  HMODULE module = nullptr;
  cgpui::VulkanWindowsFunctions table;
};
} // namespace

int main() {
  if (GetModuleHandleW(L"vulkan-1.dll") ||
      !same(published_functions(), cgpui::VulkanWindowsFunctions{})) {
    std::cerr << "Vulkan loader must not be initialized before the concurrent first admission\n";
    return 1;
  }
  std::barrier first_admission(static_cast<std::ptrdiff_t>(thread_count));
  std::array<Admission, thread_count> results;
  std::array<std::jthread, thread_count> threads;
  std::size_t started = 0;
  try {
    for (; started != thread_count; ++started) {
      threads[started] = std::jthread([&, index = started] {
        first_admission.arrive_and_wait();
        auto& result = results[index];
        const auto loaded = cgpui::ensure_vulkan_windows_loader();
        if (!loaded) { result.error = loaded.error().message; return; }
        result.table = published_functions();
        result.module = GetModuleHandleW(L"vulkan-1.dll");
        if (!complete(result.table, result.module)) {
          result.error = "successful first admission exposed incomplete or incorrect exports";
          return;
        }
        for (unsigned repeat = 0; repeat != 4; ++repeat) {
          if (!cgpui::ensure_vulkan_windows_loader() ||
              !same(result.table, published_functions()) ||
              result.module != GetModuleHandleW(L"vulkan-1.dll")) {
            result.error = "repeated concurrent admission changed published functions or module";
            return;
          }
        }
        result.success = true;
      });
    }
  } catch (const std::exception& error) {
    // Release already-created workers instead of leaving them at the barrier.
    for (std::size_t missing = started; missing != thread_count; ++missing)
      first_admission.arrive_and_drop();
    std::cerr << "could not start all loader workers: " << error.what() << '\n';
    return 2;
  }
  for (auto& thread : threads) thread.join();
  const auto final = published_functions();
  const HMODULE module = GetModuleHandleW(L"vulkan-1.dll");
  for (const auto& result : results) {
    if (!result.success || result.module != module || !same(result.table, final)) {
      std::cerr << "inconsistent concurrent Vulkan admission: " << result.error << '\n';
      return 3;
    }
  }
  if (!complete(final, module) || !cgpui::ensure_vulkan_windows_loader() ||
      !same(final, published_functions()) || module != GetModuleHandleW(L"vulkan-1.dll")) {
    std::cerr << "Vulkan module/functions did not remain available after worker destruction\n";
    return 4;
  }
  std::cout << "VULKAN_LOADER_CONCURRENCY threads=" << thread_count
            << " required=" << required_count << " stable=1 module-retained=1\n";
  return 0;
}
