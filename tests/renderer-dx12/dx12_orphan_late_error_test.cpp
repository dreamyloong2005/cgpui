#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"
#include <string_view>
#include <vector>

using namespace cgpui;
using namespace cgpui_test;

namespace {
constexpr const char marker[] = "CGPUI orphan late debug failure 20261124";
bool only_marker(ID3D12InfoQueue& queue) {
  unsigned count = 0;
  for (UINT64 index = 0; index != queue.GetNumStoredMessagesAllowedByRetrievalFilter(); ++index) {
    SIZE_T size = 0;
    if (FAILED(queue.GetMessage(index, nullptr, &size))) return false;
    std::vector<unsigned char> storage(size);
    auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
    if (FAILED(queue.GetMessage(index, message, &size))) return false;
    if (message->Severity != D3D12_MESSAGE_SEVERITY_ERROR &&
        message->Severity != D3D12_MESSAGE_SEVERITY_CORRUPTION) continue;
    if (message->Severity != D3D12_MESSAGE_SEVERITY_ERROR ||
        message->ID != D3D12_MESSAGE_ID_STRING_FROM_APPLICATION || !message->pDescription ||
        std::string_view(message->pDescription) != marker) return false;
    ++count;
  }
  return count == 1;
}
}
int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  Dx12PacingForwardApi api;
  if (!deadline.done || !window.hwnd || !pacing_begin_debug_enable())
    return dx12_fail(1, "orphan late-error fixture unavailable");
  auto created = dx12_create_renderer(window.descriptor(), api);
  if (!created) return dx12_fail(2, "orphan late-error renderer unavailable");
  auto renderer = std::move(*created);
  const auto info = renderer->backend_info();
  auto frame = pacing_begin_bounded(*renderer, api);
  if (!frame || !(*frame)->request_pixel_capture()) return dx12_fail(3, "orphan late-error frame unavailable");
  auto native = dx12_acquire_native_device(info);
  Microsoft::WRL::ComPtr<ID3D12InfoQueue> queue;
  if (!native || FAILED(native.As(&queue)) || !queue ||
      !dx12_recovery_debug_messages_safe(queue.Get())) return dx12_fail(4, "orphan live debug queue unavailable");
  queue->ClearStoredMessages();
  renderer.reset();
  (*frame)->clear({0, 1, 0, 1});
  if (FAILED(queue->AddApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR, marker)))
    return dx12_fail(5, "orphan controlled native message injection failed");
  const auto polls = api.polls;
  const auto result = (*frame)->present(); // Native submission/readback reaches its late debug check.
  const bool expected = only_marker(*queue.Get());
  queue->ClearStoredMessages(); // Only after observing every ERROR/CORRUPTION.
  if (!expected || result || result.error().code != ErrorCode::renderer_initialization_failed ||
      result.error().message.find(marker) == std::string::npos || FAILED(native->GetDeviceRemovedReason()) ||
      api.sets != 1 || api.gets != 1 || api.polls != polls || api.closes)
    return dx12_fail(6, "orphan late failure was hidden, changed epoch, retried or released a live device");
  frame->reset();
  return api.closes == 1 && api.close_ok ? 0 : dx12_fail(7, "orphan late-error cleanup did not close once");
}
