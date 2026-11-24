#pragma once
#include "dx12_dred_breadcrumb_summary_internal.hpp"
#include <array>
#include <cstddef>

namespace cgpui {
inline constexpr std::size_t dx12_dred_max_nodes = 64;
inline constexpr std::size_t dx12_dred_max_operations = 32;
inline constexpr std::uint32_t dx12_dred_history_capacity = 65536;
struct Dx12DredBreadcrumbNode {
  std::uint32_t recorded = 0, completed = 0, first_operation = 0, operation_count = 0;
  bool completed_overwritten = false, history_truncated = false;
  std::array<std::uint32_t, dx12_dred_max_operations> operations{};
};
struct Dx12DredBreadcrumbReport {
  Dx12DredBreadcrumbSummary summary;
  std::array<Dx12DredBreadcrumbNode, dx12_dred_max_nodes> nodes{};
};
// No borrowed address, string, native interface or recovery authority escapes.
} // namespace cgpui
