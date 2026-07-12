#pragma once

#include "win32_uia_provider_internal.hpp"

namespace cgpui {

bool set_win32_uia_pattern_availability_property(
    const Win32UiaProviderNode& node,
    PROPERTYID property,
    VARIANT* value);

} // namespace cgpui
