#include "dx12_dred_breadcrumb_copy_internal.hpp"
#include <algorithm>
#include <limits>

namespace cgpui {
Dx12DredCopy::Dx12DredCopy(Dx12DredMemoryReader& reader, Dx12DredBreadcrumbSummary& summary) noexcept
    : reader_(reader), summary_(summary) {}
bool Dx12DredCopy::exact(std::uintptr_t address, std::span<std::byte> destination) noexcept {
  if (!address || destination.size() > std::numeric_limits<std::uintptr_t>::max() - address) {
    summary_.status = Dx12DredBreadcrumbStatus::invalid_address;
    return false;
  }
  constexpr std::size_t max_calls = 512, max_bytes = 65536;
  if (calls_ == max_calls || destination.size() > max_bytes - bytes_) {
    summary_.status = Dx12DredBreadcrumbStatus::budget_exhausted;
    return false;
  }
  ++calls_; bytes_ += destination.size();
  const auto result = reader_.read(address, destination);
  if (!result.success || result.transferred != destination.size()) {
    summary_.status = result.transferred ? Dx12DredBreadcrumbStatus::short_read
                                        : Dx12DredBreadcrumbStatus::read_failed;
    summary_.read_error = result.error;
    return false; // Partially copied bytes never become accepted report records.
  }
  return true;
}
bool Dx12DredCopy::node(std::uintptr_t address, D3D12_AUTO_BREADCRUMB_NODE& value) noexcept {
  return exact(address, std::as_writable_bytes(std::span(&value, 1)));
}
bool Dx12DredCopy::counter(std::uintptr_t address, std::uint32_t& value) noexcept {
  return exact(address, std::as_writable_bytes(std::span(&value, 1)));
}
bool Dx12DredCopy::history(std::uintptr_t address, Dx12DredBreadcrumbNode& value) noexcept {
  if (!value.operation_count) return true;
  if (!address) { summary_.status = Dx12DredBreadcrumbStatus::invalid_address; return false; }
  const std::size_t slot = value.first_operation % dx12_dred_history_capacity;
  const std::size_t first = std::min<std::size_t>(value.operation_count, dx12_dred_history_capacity - slot);
  const auto offset = slot * sizeof(std::uint32_t);
  if (offset > std::numeric_limits<std::uintptr_t>::max() - address) {
    summary_.status = Dx12DredBreadcrumbStatus::invalid_address;
    return false;
  }
  if (!exact(address + offset, std::as_writable_bytes(std::span(value.operations).first(first)))) return false;
  const auto remaining = value.operation_count - first;
  return !remaining || exact(address,
      std::as_writable_bytes(std::span(value.operations).subspan(first, remaining)));
}
} // namespace cgpui
