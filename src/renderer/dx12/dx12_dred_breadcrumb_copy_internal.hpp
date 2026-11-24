#pragma once
#include "dx12_dred_memory_reader_internal.hpp"
#include "dx12_native_internal.hpp"

namespace cgpui {
// One sampling invocation owns its byte/call budget and all candidate storage.
class Dx12DredCopy {
 public:
  Dx12DredCopy(Dx12DredMemoryReader&, Dx12DredBreadcrumbSummary&) noexcept;
  bool exact(std::uintptr_t, std::span<std::byte>) noexcept;
  bool node(std::uintptr_t, D3D12_AUTO_BREADCRUMB_NODE&) noexcept;
  bool counter(std::uintptr_t, std::uint32_t&) noexcept;
  bool history(std::uintptr_t, Dx12DredBreadcrumbNode&) noexcept;
 private:
  Dx12DredMemoryReader& reader_;
  Dx12DredBreadcrumbSummary& summary_;
  std::size_t calls_ = 0, bytes_ = 0;
};
} // namespace cgpui
