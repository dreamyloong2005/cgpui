#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "dx12_test_support.hpp"
#include "../support/win32_desktop_test_support.hpp"

#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdlib>
#include <string_view>
#include <thread>
#include <vector>

namespace cgpui_test {

class Dx12RecoveryDeadline {
 public:
  HANDLE done = nullptr;
  explicit Dx12RecoveryDeadline(DesktopTestAccess access = DesktopTestAccess::none) : desktop_(access) {
    if (!desktop_.ready()) return;
    done = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (done) worker_ = std::thread([this] {
      if (WaitForSingleObject(done, 15000) == WAIT_TIMEOUT) {
        const char message[] = "DX12 device-recovery test timed out\n";
        DWORD written = 0;
        WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, sizeof(message) - 1, &written, nullptr);
        TerminateProcess(GetCurrentProcess(), 90); // Only this standalone test process.
      }
    });
  }
  ~Dx12RecoveryDeadline() {
    if (done) SetEvent(done);
    if (worker_.joinable()) worker_.join();
    if (done) CloseHandle(done);
  }
 private:
  Win32DesktopTestLease desktop_;
  std::thread worker_;
};

inline bool dx12_recovery_debug_messages_safe(ID3D12InfoQueue* messages, bool removing = false) {
  if (!messages) return true;
  for (UINT64 index = 0; index < messages->GetNumStoredMessagesAllowedByRetrievalFilter(); ++index) {
    SIZE_T length = 0;
    if (FAILED(messages->GetMessage(index, nullptr, &length))) return false;
    std::vector<unsigned char> storage(length);
    auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
    if (FAILED(messages->GetMessage(index, message, &length))) return false;
    if (removing && (message->ID == D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_AT_FAULT ||
        message->ID == D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_POSSIBLY_AT_FAULT ||
        message->ID == D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_NOT_AT_FAULT)) continue;
    if (message->Severity == D3D12_MESSAGE_SEVERITY_ERROR ||
        message->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION) {
      std::cerr << "unexpected recovery debug message " << message->ID << ": "
                << message->pDescription << '\n';
      return false;
    }
  }
  return true;
}

inline Microsoft::WRL::ComPtr<ID3D12Device5> dx12_acquire_native_device(
    const cgpui::RendererBackendInfo& info) {
  using Microsoft::WRL::ComPtr;
  if (!info.adapter_luid) return {};
  ComPtr<IDXGIFactory4> factory;
  ComPtr<IDXGIAdapter1> adapter;
  ComPtr<ID3D12Device5> native;
  const LUID luid{DWORD(*info.adapter_luid), LONG(*info.adapter_luid >> 32)};
  // D3D12's process/adapter singleton identifies the actual renderer device.
  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))) ||
      FAILED(factory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&adapter))) ||
      FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&native))))
    return {};
  return native;
}

inline bool dx12_recovery_debug_requested() {
  const auto enabled = [](const char* name) {
    const char* value = std::getenv(name);
    return value && std::string_view(value) == "1";
  };
  return enabled("CGPUI_DX12_DEBUG") || enabled("CGPUI_DX12_GPU_VALIDATION");
}

inline bool dx12_remove_native_device(const cgpui::RendererBackendInfo& info) {
  auto native = dx12_acquire_native_device(info);
  if (!native) return false;
  using Microsoft::WRL::ComPtr;
  ComPtr<ID3D12InfoQueue> messages;
  (void)native.As(&messages);
  if (dx12_recovery_debug_requested() && !messages) return false;
  if (messages) messages->ClearStoredMessages();
  native->RemoveDevice();
  const bool removed = FAILED(native->GetDeviceRemovedReason());
  const bool safe = dx12_recovery_debug_messages_safe(messages.Get(), true);
  // No native device or InfoQueue reference may escape into the recovery call.
  // Keeping either would itself prevent the removed singleton's replacement.
  return removed && safe;
}

inline bool dx12_recovered_native_device_clean(const cgpui::RendererBackendInfo& info) {
  auto native = dx12_acquire_native_device(info);
  if (!native || FAILED(native->GetDeviceRemovedReason())) return false;
  Microsoft::WRL::ComPtr<ID3D12InfoQueue> messages;
  (void)native.As(&messages);
  if (dx12_recovery_debug_requested() && !messages) return false;
  // This observes only the new generation; all references die before return.
  return dx12_recovery_debug_messages_safe(messages.Get());
}

} // namespace cgpui_test
