#pragma once

void refresh_output_scale();
void update_framebuffer_size();
static void handle_surface_enter(
    void* data,
    wl_surface* surface,
    wl_output* output);
static void handle_surface_leave(
    void* data,
    wl_surface* surface,
    wl_output* output);

Size logical_size_;
WaylandOutputScaleLookup output_scale_lookup_;
std::vector<wl_output*> entered_outputs_;
