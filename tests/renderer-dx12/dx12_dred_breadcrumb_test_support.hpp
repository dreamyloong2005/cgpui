#pragma once
#include "../../src/renderer/dx12/dx12_dred_breadcrumb_copy_internal.hpp"
#include <cstring>
#include <map>
#include <vector>

namespace cgpui_test {
struct DredMemoryScript final : cgpui::Dx12DredMemoryReader {
  std::map<std::uintptr_t, std::vector<std::byte>> regions;
  std::size_t calls = 0, bytes = 0, fail_call = 0, short_call = 0;
  void (*before_read)(DredMemoryScript&, std::uintptr_t) = nullptr;
  cgpui::Dx12DredReadResult read(std::uintptr_t, std::span<std::byte>) noexcept override;
  template<class T> void put(std::uintptr_t at, std::span<const T> values) {
    auto& storage = regions[at]; storage.resize(values.size_bytes());
    std::memcpy(storage.data(), values.data(), values.size_bytes());
  }
  template<class T> void put(std::uintptr_t at, const T& value) { put(at, std::span<const T>(&value, 1)); }
};
inline constexpr std::uintptr_t dred_node_address = 0x10000;
inline constexpr std::uintptr_t dred_counter_address = 0x20000;
inline constexpr std::uintptr_t dred_history_address = 0x30000;
D3D12_AUTO_BREADCRUMB_NODE dred_memory_node(std::uint32_t count = 4, std::uintptr_t next = 0);
void dred_memory_fixture(DredMemoryScript&);
int dred_breadcrumb_fail(int, const char*);
} // namespace cgpui_test
