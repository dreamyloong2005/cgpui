#include "win32_composition_host_internal.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dcomp.h>
#include <wrl/client.h>

namespace cgpui {

bool win32_composition_host_available() {
  static const bool available = [] {
    Microsoft::WRL::ComPtr<IDCompositionDevice> composition;
    // A rendering device is optional for a host of targets and visuals.
    // Creating GPU-backed content is the selected renderer's responsibility.
    return SUCCEEDED(DCompositionCreateDevice(
        nullptr, IID_PPV_ARGS(&composition)));
  }();
  return available;
}

} // namespace cgpui
