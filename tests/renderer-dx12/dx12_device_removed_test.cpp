#define NOMINMAX
#include "dx12_test_support.hpp"

#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdlib>
#include <string_view>
#include <thread>
#include <vector>

using namespace cgpui;
using namespace cgpui_test;
using Microsoft::WRL::ComPtr;

namespace {
class Deadline {
 public:
  HANDLE done = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  Deadline() {
    if (done) worker_ = std::thread([this] {
      if (WaitForSingleObject(done, 15000) == WAIT_TIMEOUT) {
        const char message[] = "DX12 device-removal test timed out\n";
        DWORD written = 0;
        WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, sizeof(message) - 1, &written, nullptr);
        TerminateProcess(GetCurrentProcess(), 90); // Only this standalone test process.
      }
    });
  }
  ~Deadline() {
    if (done) SetEvent(done);
    if (worker_.joinable()) worker_.join();
    if (done) CloseHandle(done);
  }
 private:
  std::thread worker_;
};

bool no_unsafe_debug_errors(ID3D12InfoQueue* messages) {
  if (!messages) return true; // Non-debug matrices cannot supply debug-layer evidence.
  for (UINT64 index = 0; index < messages->GetNumStoredMessagesAllowedByRetrievalFilter(); ++index) {
    SIZE_T length = 0;
    if (FAILED(messages->GetMessage(index, nullptr, &length))) return false;
    std::vector<unsigned char> storage(length);
    auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
    if (FAILED(messages->GetMessage(index, message, &length))) return false;
    // Explicit RemoveDevice legitimately reports device removal. Do not hide
    // lifetime/descriptor/deleted-while-in-use errors under a generic filter.
    if (message->ID == D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_AT_FAULT ||
        message->ID == D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_POSSIBLY_AT_FAULT ||
        message->ID == D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_NOT_AT_FAULT) continue;
    if (message->Severity == D3D12_MESSAGE_SEVERITY_ERROR ||
        message->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION) {
      std::cerr << "unexpected removal debug message " << message->ID << ": "
                << message->pDescription << '\n';
      return false;
    }
  }
  return true;
}

void paint_resources(RenderFrame& frame, const ImageAsset& image) {
  frame.clear({0, 0, 1, 1});
  frame.draw_image({.bounds = {{4, 4}, {32, 32}}, .asset = describe_image_asset(image)});
  frame.draw_box_shadow({.rect = {{40, 40}, {16, 16}},
      .shadow = {.color = {0, 0, 0, 1}, .offset = {-8, -8}, .blur_radius = 4}});
  frame.draw_backdrop_blur({.rect = {{8, 8}, {40, 40}}, .blur_radius = 4});
}
} // namespace

int main() {
  Deadline deadline;
  if (!deadline.done) return dx12_fail(1, "device-removal watchdog unavailable");
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(2, "device-removal window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(3, renderer.error().message.c_str());
  const auto info = (*renderer)->backend_info();
  if (!info.adapter_luid) return dx12_fail(4, "device-removal adapter LUID unavailable");
  ComPtr<IDXGIFactory4> factory;
  ComPtr<IDXGIAdapter1> adapter;
  ComPtr<ID3D12Device5> native;
  const LUID luid{DWORD(*info.adapter_luid), LONG(*info.adapter_luid >> 32)};
  // D3D12 devices are process/adapter singletons. This public native API gets
  // the renderer's actual device without exposing a production test hook.
  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))) ||
      FAILED(factory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&adapter))) ||
      FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&native))))
    return dx12_fail(5, "same-adapter ID3D12Device5 unavailable; removal was not tested");
  ComPtr<ID3D12InfoQueue> messages;
  (void)native.As(&messages);
  const auto enabled = [](const char* name) {
    const char* value = std::getenv(name);
    return value && std::string_view(value) == "1";
  };
  if ((enabled("CGPUI_DX12_DEBUG") || enabled("CGPUI_DX12_GPU_VALIDATION")) && !messages)
    return dx12_fail(14, "requested device-removal debug validation has no InfoQueue");
  const ImageAsset image{.id = {8301}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  auto warmup = (*renderer)->begin_frame();
  if (!warmup) return dx12_fail(6, "device-removal warmup begin failed");
  (*warmup)->upload_image(image);
  paint_resources(**warmup, image);
  if (!(*warmup)->request_pixel_capture() || !(*warmup)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 60, 60, {0, 0, 255, 255}))
    return dx12_fail(7, "device-removal resource warmup failed");
  warmup->reset();
  auto frame = (*renderer)->begin_frame();
  if (!frame) return dx12_fail(8, "device-removal frame begin failed");
  paint_resources(**frame, image); // Reuses the previously realized image snapshot.
  if (!(*frame)->request_pixel_capture()) return dx12_fail(9, "removal capture request failed");
  if (messages) messages->ClearStoredMessages();
  native->RemoveDevice();
  if (SUCCEEDED(native->GetDeviceRemovedReason()))
    return dx12_fail(10, "native device was not actually removed");
  const auto presented = (*frame)->present();
  if (presented || presented.error().code != ErrorCode::renderer_initialization_failed)
    return dx12_fail(11, "removed device did not report its terminal renderer error");
  if ((*renderer)->last_frame_pixels() || (*renderer)->begin_frame() || (*renderer)->resize({65, 49}, {1}))
    return dx12_fail(12, "removed device retained successful capture or accepted further work");
  frame->reset();
  renderer->reset();
  // This removes an idle native device, not a simulated live-queue wait failure.
  // The independent synchronization fault tests must prove that separate case.
  return no_unsafe_debug_errors(messages.Get()) ? 0 : dx12_fail(13, "unsafe removal cleanup");
}
