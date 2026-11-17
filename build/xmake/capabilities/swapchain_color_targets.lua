-- Swapchain color encoding: Color components are sRGB-encoded and blended
-- in that space, so the Vulkan swapchain prefers a UNORM format.
local public_includedirs = "include"

if is_plat("windows", "linux") then
    target("vulkan_surface_format_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_surface_format_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "src/renderer/vulkan"))
        if is_plat("windows") then
            add_packages("vulkansdk")
        else
            add_syslinks("vulkan")
        end
        add_tests("default")
end

if is_plat("windows") then
    target("vulkan_color_encoding_pixel_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_color_encoding_pixel_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "tests/renderer"))
        add_syslinks("user32")
        add_tests("default")
end
