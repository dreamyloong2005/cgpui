#include "win32_system_application_internal.hpp"
#include "win32_dpi_awareness_internal.hpp"
#include "../../renderer/win32_composition/win32_composition_presenter_internal.hpp"

#include <utility>

namespace cgpui {

Win32SystemApplication::Win32SystemApplication()
    : dpi_awareness_(win32_declare_dpi_awareness()),
      appearance_monitor_(GetModuleHandleW(nullptr)) {
  (void)appearance_monitor_.ensure_started();
}

SystemAppearance Win32SystemApplication::system_appearance() const {
  return win32_read_system_appearance();
}

void Win32SystemApplication::set_system_appearance_callback(
    SystemAppearanceCallback callback) {
  appearance_monitor_.set_callback(std::move(callback));
  (void)appearance_monitor_.ensure_started();
}

PlatformDpiAwarenessState Win32SystemApplication::dpi_awareness() const {
  return dpi_awareness_;
}

bool Win32SystemApplication::popup_windows_supported() const {
  return win32_composition_available();
}

} // namespace cgpui
