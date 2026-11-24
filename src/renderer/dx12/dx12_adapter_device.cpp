#include "dx12_device_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_recovery_ticket_internal.hpp"
#include <utility>

namespace cgpui {
HRESULT dx12_open_adapter_device(Dx12State& state) {
  dx12_observe_dred_status(state);
  state.dred_status.close_local_lease();
  state.device.Reset();
  auto& tickets = dx12_recovery_tickets();
  tickets.released(state.recovery_lease); // A reset, never weak-owner expiration, acknowledges release.
  DXGI_ADAPTER_DESC1 description{};
  HRESULT result = state.adapter->GetDesc1(&description);
  if (FAILED(result)) return result;
  const auto luid = (std::uint64_t(std::uint32_t(description.AdapterLuid.HighPart)) << 32) |
                    description.AdapterLuid.LowPart;
  if (!state.window_thread_id) state.window_thread_id = GetWindowThreadProcessId(state.hwnd, nullptr);
  auto lease = tickets.begin_attach(luid, state.weak_from_this(),
      state.window_thread_id, state.recovery_work_token);
  if (!lease) return HRESULT_FROM_WIN32(ERROR_BUSY);
  state.recovery_lease = std::move(lease);
  result = D3D12CreateDevice(state.adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                    IID_PPV_ARGS(&state.device));
  if (FAILED(result)) {
    state.device.Reset();
    tickets.released(state.recovery_lease);
    return result;
  }
  tickets.attached(state.recovery_lease);
  state.dred_status.open_local_lease();
  D3D12_FEATURE_DATA_SHADER_MODEL model{D3D_SHADER_MODEL_6_0};
  result = state.device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &model, sizeof(model));
  const HRESULT removal = state.device->GetDeviceRemovedReason();
  if (FAILED(removal)) result = removal;
  if (FAILED(result) || model.HighestShaderModel < D3D_SHADER_MODEL_6_0) {
    dx12_observe_dred_status(state);
    state.dred_status.close_local_lease();
    state.device.Reset();
    tickets.released(state.recovery_lease);
    return FAILED(result) ? result : DXGI_ERROR_UNSUPPORTED;
  }
  return S_OK;
}

Result<void> dx12_select_pinned_adapter(Dx12State& state, std::uint64_t value) {
  const LUID luid{static_cast<DWORD>(value), static_cast<LONG>(value >> 32)};
  HRESULT result = state.factory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&state.adapter));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "pinned adapter lookup"));
  result = dx12_open_adapter_device(state);
  if (result == HRESULT_FROM_WIN32(ERROR_BUSY)) {
    auto& tickets = dx12_recovery_tickets();
    auto wave = tickets.request(state.recovery_lease);
    if (tickets.poll(wave) == Dx12RecoveryTicketStatus::pending) {
      state.recovery_ticket = std::move(wave);
      return std::unexpected(Error{.code = ErrorCode::renderer_recovery_pending,
          .message = "DX12 pinned attachment is waiting for another removal wave"});
    }
  }
  if (FAILED(result)) return std::unexpected(dx12_error(result, "pinned adapter device creation"));
  return {};
}
} // namespace cgpui
