#include "dx12_adapter_option_internal.hpp"
#include <charconv>

namespace cgpui {
Result<Dx12AdapterOption> dx12_parse_adapter_option(std::string_view value) {
  if (value.empty()) return Dx12AdapterOption{};
  if (value == "warp") return Dx12AdapterOption{Dx12AdapterKind::warp};
  if (value == "high_performance") return Dx12AdapterOption{Dx12AdapterKind::high_performance};
  if (value == "minimum_power") return Dx12AdapterOption{Dx12AdapterKind::minimum_power};
  std::uint32_t index = 0;
  const auto parsed = std::from_chars(value.data(), value.data() + value.size(), index);
  if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
    return std::unexpected(Error{ErrorCode::invalid_argument,
        "CGPUI_DX12_ADAPTER must be warp, high_performance, minimum_power or an adapter index"});
  return Dx12AdapterOption{Dx12AdapterKind::index, index};
}
} // namespace cgpui
