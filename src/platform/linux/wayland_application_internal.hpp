#pragma once

#include "wayland_application_core_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<PlatformApplication>> create_wayland_application();

}  // namespace cgpui
