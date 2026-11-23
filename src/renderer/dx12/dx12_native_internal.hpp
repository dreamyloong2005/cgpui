#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "cgpui/renderer/renderer.hpp"
#include "cgpui/renderer/renderer_backend_info.hpp"

namespace cgpui {
template <typename T> using Dx12Ptr = Microsoft::WRL::ComPtr<T>;
Error dx12_error(HRESULT result, const char* operation);
Error dx12_unsupported(const char* feature);
Result<void> dx12_validate_extent(Size extent, DpiScale scale);

class Dx12Event {
 public:
  Dx12Event() = default;
  ~Dx12Event();
  Dx12Event(const Dx12Event&) = delete;
  Dx12Event& operator=(const Dx12Event&) = delete;
  HANDLE value = nullptr;
};
} // namespace cgpui
