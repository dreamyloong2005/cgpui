#pragma once

void set_window_cursor(WaylandWindow& window, CursorShape cursor_shape);
void record_cursor_theme_state(
    CursorShape cursor_shape,
    std::uint32_t serial);
void apply_cursor_for(WaylandWindow* window);
