#include "macos_window_internal.hpp"

namespace cgpui {

MacOSTextServicesDiagnostics macos_text_services_diagnostics(
    const PlatformWindow& window) {
  const auto* macos_window = dynamic_cast<const MacOSWindow*>(&window);
  return macos_window == nullptr ? MacOSTextServicesDiagnostics{}
                                 : macos_window->text_services_diagnostics();
}

}  // namespace cgpui
