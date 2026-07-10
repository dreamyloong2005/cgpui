#pragma once

Result<void> initialize(
    wl_compositor* compositor,
    xdg_wm_base* shell,
    zxdg_decoration_manager_v1* decoration_manager,
    const WindowDescriptor& descriptor);
