#include "cgpui/platform/platform.hpp"

#include <memory>
#include <string>
#include <utility>

namespace cgpui {

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::unexpected(Error{
      .code = ErrorCode::unsupported_platform,
      .message = "No platform implementation is enabled for this host",
  });
}

} // namespace cgpui
