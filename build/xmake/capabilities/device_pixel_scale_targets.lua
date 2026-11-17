-- Device pixel scale: the Vulkan renderer converts logical-pixel RenderFrame
-- draws to device pixels with the surface DpiScale.
local public_includedirs = "include"

target("device_pixel_scale_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/device_pixel_scale_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

if is_plat("windows", "linux") then
    target("vulkan_device_pixels_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_device_pixels_test.cpp"))
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
    target("vulkan_scale_pixel_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_scale_pixel_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "tests/renderer"))
        add_syslinks("user32")
        add_tests("default")
end

if is_plat("windows") then
    target("win32_ime_placement_scale_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/win32_ime_placement_scale_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(path.join(os.projectdir(), public_includedirs))
        add_syslinks("user32", "imm32")
        add_tests("default")
end
