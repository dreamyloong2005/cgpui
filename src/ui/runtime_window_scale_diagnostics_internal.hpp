#pragma once

#include "cgpui/platform/platform_diagnostics.hpp"

namespace cgpui {

[[nodiscard]] PlatformDiagnosticEvent window_scale_diagnostic(
    const WindowResized& event,
    bool succeeded);

} // namespace cgpui
