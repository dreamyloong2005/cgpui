#pragma once

WaylandWindow(
    wl_display* display,
    PlatformEventCallback callback,
    WindowState state,
    WaylandOutputScaleLookup output_scale_lookup,
    WaylandWindowScaleCallback scale_changed);
Result<void> initialize(
    wl_compositor* compositor,
    xdg_wm_base* shell,
    zxdg_decoration_manager_v1* decoration_manager,
    WaylandFractionalScaleGlobals fractional_scale,
    xdg_toplevel* parent,
    const WindowDescriptor& descriptor);
