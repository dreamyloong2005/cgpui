#pragma once

void configure_data_device_lookup();
void configure_text_input_lookup();

static void handle_global(
    void* data,
    wl_registry* registry,
    std::uint32_t name,
    const char* interface,
    std::uint32_t version);
static void handle_global_remove(
    void* data,
    wl_registry* registry,
    std::uint32_t name);
static void handle_shell_ping(
    void* data,
    xdg_wm_base* shell,
    std::uint32_t serial);

void register_window(WaylandWindow* window);
void unregister_window(WaylandWindow* window);
[[nodiscard]] WaylandWindow* find_window(wl_surface* surface) const;
