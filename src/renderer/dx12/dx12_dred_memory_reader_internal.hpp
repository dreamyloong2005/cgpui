#pragma once
#include "dx12_dred_breadcrumb_model_internal.hpp"
#include <cstdint>
#include <span>

namespace cgpui {
struct Dx12DredReadResult {
  bool success = false;
  std::size_t transferred = 0;
  std::uint32_t error = 0;
};
class Dx12DredMemoryReader {
 public:
  virtual ~Dx12DredMemoryReader() = default;
  virtual Dx12DredReadResult read(std::uintptr_t address, std::span<std::byte>) noexcept = 0;
};
Dx12DredMemoryReader& dx12_dred_process_reader() noexcept;
Dx12DredBreadcrumbReport dx12_sample_dred_breadcrumbs(
    std::uintptr_t head, Dx12DredMemoryReader&) noexcept;
} // namespace cgpui
