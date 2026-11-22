#include "vulkan_popup_support_internal.hpp"

namespace cgpui {

bool vulkan_popup_windows_supported() {
  // Windows uses the private Vulkan/D3D11 composition path. The platform
  // must also support popups; actual creation/placement failure falls back
  // in the widgets rather than disabling a whole rendering backend.
  return true;
}

}  // namespace cgpui
