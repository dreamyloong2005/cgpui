#pragma once
#include "cgpui/core/error.hpp"
#include <cstdint>
#include <string_view>

namespace cgpui {
enum class Dx12AdapterKind { automatic, index, warp, high_performance, minimum_power };
struct Dx12AdapterOption {
  Dx12AdapterKind kind = Dx12AdapterKind::automatic;
  std::uint32_t index = 0;
};
// Construction-only diagnostic setting. Every nonempty valid option is pinned
// policy: it must not authorize automatic Vulkan fallback.
Result<Dx12AdapterOption> dx12_parse_adapter_option(std::string_view value);
} // namespace cgpui
