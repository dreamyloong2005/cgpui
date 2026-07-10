#include "cgpui/platform/platform_application.hpp"

#include <utility>

namespace cgpui {

Result<std::unique_ptr<PlatformWindow>> PlatformApplication::create_child_window(
    const WindowDescriptor& descriptor,
    PlatformWindow&,
    PlatformEventCallback callback) {
  return create_window(descriptor, std::move(callback));
}

} // namespace cgpui
