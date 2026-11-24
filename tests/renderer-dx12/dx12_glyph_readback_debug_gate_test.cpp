#include "dx12_glyph_native_test_support.hpp"
#include <d3d12sdklayers.h>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

using namespace cgpui;
using namespace cgpui_test;
namespace {
constexpr const char* injected = "CGPUI glyph readback validation sentinel";
bool only_expected_error(ID3D12InfoQueue& queue) {
  bool found = false;
  const auto count = queue.GetNumStoredMessagesAllowedByRetrievalFilter();
  for (UINT64 i = 0; i < count; ++i) {
    SIZE_T bytes = 0;
    if (FAILED(queue.GetMessage(i, nullptr, &bytes)) || bytes < sizeof(D3D12_MESSAGE)) return false;
    std::vector<unsigned char> storage(bytes);
    auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
    if (FAILED(queue.GetMessage(i, message, &bytes))) return false;
    if (message->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION) return false;
    if (message->Severity != D3D12_MESSAGE_SEVERITY_ERROR) continue;
    if (!message->pDescription ||
        std::string_view(message->pDescription).find(injected) == std::string_view::npos) return false;
    found = true;
  }
  return found;
}
}
int main() {
  // Dedicated process: the debug layer must be enabled before its first device.
  // Preserve any requested GPU validation and adapter selection.
  if (_putenv_s("CGPUI_DX12_DEBUG", "1")) return dx12_fail(1, "debug injection environment");
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(2, "debug injection native fixture unavailable; not semantic RED");
  glyph_add_record(*state, 1);
  auto a = glyph_publish(*state);
  if (!a) return dx12_fail(3, "debug injection snapshot");
  auto draw = glyph_draw(a);
  if (!glyph_submit(*state, std::span{&draw, 1}) || !glyph_pixels(*state, *a))
    return dx12_fail(4, "debug injection completed source");
  Dx12Ptr<ID3D12InfoQueue> queue;
  if (FAILED(state->device.As(&queue)) ||
      FAILED(queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, FALSE)) ||
      !dx12_check_debug_errors(*state) ||
      FAILED(queue->AddApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR, injected)))
    return dx12_fail(5, "real info-queue injection unavailable; not semantic RED");
  auto* native = state->texture_renderer->native_textures.at(a.get()).resource.Get();
  const auto denied = glyph_readback(*state, *native); // Real copy/wait/Map/Unmap, then the shared gate.
  if (denied || denied.error().code != ErrorCode::renderer_initialization_failed ||
      denied.error().message.find(injected) == std::string::npos || !only_expected_error(*queue.Get()))
    return dx12_fail(6, "readback accepted or hid native ERROR; ordinary failure, never exit40");
  // Clear only after proving every queued ERROR is the intentional injection.
  // A different ERROR/CORRUPTION above is a failure, never masked as expected noise.
  queue->ClearStoredMessages();
  auto clean = glyph_readback(*state, *native);
  if (!clean || *clean != a->pixels || !dx12_check_debug_errors(*state))
    return dx12_fail(7, "clean readback after explicit injected-message cleanup");
  std::cout << "glyph readback debug rejection control passed\n";
  return 0;
}
