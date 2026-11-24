#include "dx12_dred_breadcrumb_copy_internal.hpp"
#include <algorithm>

namespace cgpui {
namespace {
std::uintptr_t address(const void* value) noexcept { return reinterpret_cast<std::uintptr_t>(value); }
bool same(const D3D12_AUTO_BREADCRUMB_NODE& left, const D3D12_AUTO_BREADCRUMB_NODE& right) noexcept {
  return left.BreadcrumbCount == right.BreadcrumbCount && address(left.pNext) == address(right.pNext) &&
      address(left.pCommandHistory) == address(right.pCommandHistory) &&
      address(left.pLastBreadcrumbValue) == address(right.pLastBreadcrumbValue);
}
void window(Dx12DredBreadcrumbNode& out) noexcept {
  const auto oldest = out.recorded > dx12_dred_history_capacity ? out.recorded - dx12_dred_history_capacity : 0;
  out.completed_overwritten = out.completed && out.completed <= oldest;
  const auto around = out.completed > 16 ? out.completed - 16 : 0;
  out.first_operation = std::max(oldest, around);
  out.operation_count = std::min<std::uint32_t>(
      static_cast<std::uint32_t>(dx12_dred_max_operations), out.recorded - out.first_operation);
  out.history_truncated = out.operation_count < out.recorded;
}
}
Dx12DredBreadcrumbReport dx12_sample_dred_breadcrumbs(
    std::uintptr_t current, Dx12DredMemoryReader& reader) noexcept {
  Dx12DredBreadcrumbReport report;
  auto& summary = report.summary;
  if (!current) { summary.status = Dx12DredBreadcrumbStatus::observed_empty; return report; }
  Dx12DredCopy copy(reader, summary);
  std::array<std::uintptr_t, dx12_dred_max_nodes> visited{};
  for (std::size_t index = 0; current; ++index) {
    if (std::find(visited.begin(), visited.begin() + index, current) != visited.begin() + index) {
      summary.status = Dx12DredBreadcrumbStatus::cycle; break;
    }
    if (index == visited.size()) { summary.status = Dx12DredBreadcrumbStatus::truncated; break; }
    visited[index] = current;
    D3D12_AUTO_BREADCRUMB_NODE first{}, last{};
    Dx12DredBreadcrumbNode candidate;
    if (!copy.node(current, first)) break;
    candidate.recorded = first.BreadcrumbCount;
    if (first.pLastBreadcrumbValue) {
      if (!copy.counter(address(first.pLastBreadcrumbValue), candidate.completed)) break;
    } else if (candidate.recorded) {
      summary.status = Dx12DredBreadcrumbStatus::invalid_address; break;
    }
    if (candidate.completed > candidate.recorded) {
      summary.status = Dx12DredBreadcrumbStatus::inconsistent; break;
    }
    window(candidate);
    if (!copy.history(address(first.pCommandHistory), candidate) || !copy.node(current, last)) break;
    std::uint32_t completed = 0;
    if (last.pLastBreadcrumbValue && !copy.counter(address(last.pLastBreadcrumbValue), completed)) break;
    if (!same(first, last) || completed != candidate.completed) {
      summary.status = Dx12DredBreadcrumbStatus::inconsistent; break;
    }
    // Equal re-reads detect some mutation, not ABA or an atomic/coherent snapshot.
    report.nodes[summary.nodes++] = candidate;
    summary.operations += candidate.operation_count;
    summary.history_truncated |= candidate.history_truncated;
    summary.status = Dx12DredBreadcrumbStatus::sampled;
    current = address(first.pNext);
  }
  return report;
}
} // namespace cgpui
