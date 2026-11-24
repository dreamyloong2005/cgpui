#pragma once
#include "dx12_dred_status_internal.hpp"
#include <array>
#include <cstddef>

namespace cgpui {
struct Dx12DredStatusMessage {
  std::array<wchar_t, 768> text{};
  std::size_t used = 0;
  bool truncated = false;
};
Dx12DredStatusMessage dx12_format_dred_status(const Dx12DredStatus& status) noexcept;
void dx12_emit_dred_status(const Dx12DredStatus& status) noexcept;
} // namespace cgpui
