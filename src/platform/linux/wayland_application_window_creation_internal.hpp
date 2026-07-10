#pragma once

Result<std::unique_ptr<PlatformWindow>> create_window_with_parent(
    const WindowDescriptor& descriptor,
    PlatformEventCallback callback,
    xdg_toplevel* parent);
