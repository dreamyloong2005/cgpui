#pragma once

void decoration_configured(std::uint32_t mode);
static void handle_decoration_configure(
    void* data,
    zxdg_toplevel_decoration_v1* decoration,
    std::uint32_t mode);

zxdg_toplevel_decoration_v1* decoration_ = nullptr;
