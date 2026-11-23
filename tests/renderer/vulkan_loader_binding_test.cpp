#include "vulkan_loader_windows_internal.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>

namespace {
constexpr const char* required_symbols[]{
#define CGPUI_VULKAN_ENTRY(name) #name,
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
};

HMODULE fake_module() { return reinterpret_cast<HMODULE>(std::uintptr_t{1}); }
const char* missing_symbol = nullptr;
unsigned resolver_calls = 0, unexpected_calls = 0;
bool wrong_module = false, missing_seen = false, unknown_symbol = false;

// Its address is only a non-null resolver result. No Vulkan call may execute it.
INT_PTR WINAPI dummy_export() { ++unexpected_calls; return 0; }

FARPROC WINAPI resolve_symbol(HMODULE module, LPCSTR name) {
  ++resolver_calls;
  wrong_module = wrong_module || module != fake_module();
  bool known = false;
  for (const char* required : required_symbols) known = known || std::strcmp(name, required) == 0;
  unknown_symbol = unknown_symbol || !known;
  if (missing_symbol && std::strcmp(name, missing_symbol) == 0) {
    missing_seen = true;
    return nullptr;
  }
  return &dummy_export;
}

void reset_resolver(const char* missing = nullptr) {
  missing_symbol = missing;
  resolver_calls = 0;
  wrong_module = missing_seen = unknown_symbol = false;
}

bool globals_unpublished() {
#define CGPUI_VULKAN_ENTRY(name) if (cgpui::name != nullptr) return false;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
  return true;
}

bool complete_table(const cgpui::VulkanWindowsFunctions& table) {
#define CGPUI_VULKAN_ENTRY(name) \
  if (!table.name || reinterpret_cast<FARPROC>(table.name) != &dummy_export) return false;
#include "vulkan_loader_windows_symbols.inc"
#undef CGPUI_VULKAN_ENTRY
  return true;
}

int fail(int code, std::string_view reason) {
  std::cerr << "Vulkan loader binding: " << reason << '\n';
  return code;
}
} // namespace

int main() {
  // This process never calls ensure: resolving a candidate must not publish it.
  if (!globals_unpublished()) return fail(1, "loader functions were eagerly published");
  reset_resolver();
  const auto no_module = cgpui::resolve_vulkan_windows_functions(nullptr, resolve_symbol);
  if (no_module || no_module.error().code != cgpui::ErrorCode::invalid_argument || resolver_calls)
    return fail(2, "null module was not rejected before resolving");
  const auto no_resolver = cgpui::resolve_vulkan_windows_functions(fake_module(), nullptr);
  const auto neither = cgpui::resolve_vulkan_windows_functions(nullptr, nullptr);
  if (no_resolver || neither || no_resolver.error().code != cgpui::ErrorCode::invalid_argument ||
      neither.error().code != cgpui::ErrorCode::invalid_argument || resolver_calls || !globals_unpublished())
    return fail(3, "null resolver was accepted or invalid input published functions");

  for (const char* missing : required_symbols) {
    reset_resolver(missing);
    const auto bound = cgpui::resolve_vulkan_windows_functions(fake_module(), resolve_symbol);
    if (bound || bound.error().code != cgpui::ErrorCode::renderer_initialization_failed ||
        bound.error().message.find(missing) == std::string::npos || !missing_seen ||
        !resolver_calls || wrong_module || unknown_symbol || unexpected_calls || !globals_unpublished()) {
      std::cerr << "missing required export: " << missing << '\n';
      return fail(4, "missing export did not fail transactionally with its name");
    }
  }

  reset_resolver();
  const auto complete = cgpui::resolve_vulkan_windows_functions(fake_module(), resolve_symbol);
  if (!complete || !complete_table(*complete) || resolver_calls != std::size(required_symbols) ||
      wrong_module || unknown_symbol || unexpected_calls || !globals_unpublished())
    return fail(5, "complete candidate was not typed, complete and unpublished");
  std::cout << "VULKAN_BINDING required=" << std::size(required_symbols)
            << " missing-each=refused globals=unpublished calls-to-exports=0\n";
  return 0;
}
