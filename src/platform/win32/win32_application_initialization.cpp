#include "win32_application_initialization_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return create_win32_application(&CreateThreadpoolTimer);
}

} // namespace cgpui
