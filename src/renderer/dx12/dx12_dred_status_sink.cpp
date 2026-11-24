#include "dx12_dred_status_sink_internal.hpp"
#include "dx12_native_internal.hpp"

namespace cgpui {
void dx12_emit_dred_status(const Dx12DredStatus& status) noexcept {
  const auto message = dx12_format_dred_status(status);
  OutputDebugStringW(message.text.data());
}
} // namespace cgpui
