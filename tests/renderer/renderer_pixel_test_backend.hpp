#pragma once

#include "cgpui/renderer/renderer_frame.hpp"

#include <string_view>

namespace cgpui_test {
[[nodiscard]] const char* pixel_backend_name(cgpui::RendererBackend backend);
[[nodiscard]] cgpui::Result<cgpui::RendererBackend> parse_pixel_test_backend(
    std::string_view requested);
[[nodiscard]] cgpui::Result<void> validate_pixel_backend_info(
    cgpui::RendererBackend expected, const cgpui::RendererBackendInfo& info,
    bool require_software = false);
[[nodiscard]] cgpui::Result<std::unique_ptr<cgpui::Renderer>> create_pixel_test_renderer(
    const cgpui::RenderSurfaceDescriptor& descriptor);
void report_pixel_test_error(std::string_view stage, const cgpui::Error& error);
[[nodiscard]] bool pixel_test_ok(
    const cgpui::Result<void>& result, std::string_view stage);
} // namespace cgpui_test
