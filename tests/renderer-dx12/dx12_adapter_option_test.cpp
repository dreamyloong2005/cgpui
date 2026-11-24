#include "../../src/renderer/dx12/dx12_adapter_option_internal.hpp"
#include "../../src/renderer/dx12/dx12_initialization_failure_internal.hpp"
#include <cstdlib>
#include <string>
#include <utility>

using namespace cgpui;
namespace {
struct Environment {
  const char* name;
  std::string old;
  explicit Environment(const char* key) : name(key) {
    if (const auto* value = std::getenv(key)) old = value;
    _putenv_s(name, "");
  }
  ~Environment() { _putenv_s(name, old.c_str()); }
};
}
int main() {
  for (const auto& [text, kind] : {std::pair{"", Dx12AdapterKind::automatic},
      {"warp", Dx12AdapterKind::warp}, {"high_performance", Dx12AdapterKind::high_performance},
      {"minimum_power", Dx12AdapterKind::minimum_power}}) {
    const auto parsed = dx12_parse_adapter_option(text);
    if (!parsed || parsed->kind != kind || parsed->index != 0) return 1;
  }
  for (const auto& [text, index] : {std::pair{"0", 0U}, {"0007", 7U}, {"4294967295", 0xffffffffU}}) {
    const auto parsed = dx12_parse_adapter_option(text);
    if (!parsed || parsed->kind != Dx12AdapterKind::index || parsed->index != index) return 2;
  }
  for (const auto* text : {"auto", "high-performance", "minimum-power", "HIGH_PERFORMANCE", "WARP",
      " high_performance", "minimum_power ", "1 ", "-1", "+1", "1x", "0x1", "4294967296"}) {
    const auto parsed = dx12_parse_adapter_option(text);
    if (parsed || parsed.error().code != ErrorCode::invalid_argument) return 3;
  }
  Environment adapter("CGPUI_DX12_ADAPTER"), debug("CGPUI_DX12_DEBUG"), gpu("CGPUI_DX12_GPU_VALIDATION");
  if (!dx12_initialization_fallback_allowed().value_or(false)) return 4;
  for (const auto* text : {"warp", "0", "4294967295", "high_performance", "minimum_power"}) {
    if (_putenv_s(adapter.name, text)) return 5;
    const auto allowed = dx12_initialization_fallback_allowed();
    if (!allowed || *allowed) return 6;
  }
  if (_putenv_s(adapter.name, " high_performance")) return 7;
  const auto invalid = dx12_initialization_fallback_allowed();
  return !invalid && invalid.error().code == ErrorCode::invalid_argument ? 0 : 8;
}
