#include "dx12_dred_breadcrumb_test_support.hpp"
#include <array>
#include <iostream>

namespace cgpui_test {
cgpui::Dx12DredReadResult DredMemoryScript::read(std::uintptr_t at, std::span<std::byte> output) noexcept {
  ++calls; bytes += output.size();
  if (before_read) before_read(*this, at);
  if (calls == fail_call) return {false, 0, 998};
  for (const auto& [base, storage] : regions) {
    if (at < base || at - base > storage.size() || output.size() > storage.size() - (at - base)) continue;
    const auto count = calls == short_call ? output.size() - 1 : output.size();
    std::memcpy(output.data(), storage.data() + (at - base), count);
    return {true, count, 0};
  }
  return {false, 0, 487};
}
D3D12_AUTO_BREADCRUMB_NODE dred_memory_node(std::uint32_t count, std::uintptr_t next) {
  D3D12_AUTO_BREADCRUMB_NODE node{};
  node.BreadcrumbCount = count;
  node.pLastBreadcrumbValue = reinterpret_cast<const UINT*>(dred_counter_address);
  node.pCommandHistory = reinterpret_cast<const D3D12_AUTO_BREADCRUMB_OP*>(dred_history_address);
  node.pNext = reinterpret_cast<const D3D12_AUTO_BREADCRUMB_NODE*>(next);
  // These fields are deliberately invalid: the copier must never follow them.
  node.pCommandListDebugNameA = reinterpret_cast<const char*>(1);
  node.pCommandListDebugNameW = reinterpret_cast<const wchar_t*>(1);
  node.pCommandQueueDebugNameA = reinterpret_cast<const char*>(1);
  node.pCommandQueueDebugNameW = reinterpret_cast<const wchar_t*>(1);
  node.pCommandList = reinterpret_cast<ID3D12GraphicsCommandList*>(1);
  node.pCommandQueue = reinterpret_cast<ID3D12CommandQueue*>(1);
  return node;
}
void dred_memory_fixture(DredMemoryScript& script) {
  script.put(dred_node_address, dred_memory_node());
  script.put(dred_counter_address, std::uint32_t{2});
  const std::array<std::uint32_t, 4> ops{11, 22, 33, 44};
  script.put(dred_history_address, std::span<const std::uint32_t>(ops));
}
int dred_breadcrumb_fail(int code, const char* message) {
  std::cerr << "DRED bounded breadcrumb: " << message << '\n'; return code;
}
} // namespace cgpui_test
