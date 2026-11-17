-- Window backdrop material (C3): DWM system backdrops on Windows, compositor
-- blur protocols on Wayland, and Vulkan surface composition reporting.
local public_includedirs = "include"

target("window_backdrop_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/window_backdrop_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("core_window_backdrop_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/core_window_backdrop_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("platform_window_backdrop_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/platform_window_backdrop_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("renderer_surface_composition_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/renderer_surface_composition_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("window_runtime_backdrop_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/window_runtime_backdrop_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs), path.join(os.projectdir(), "src/ui"))
    add_tests("default")

if is_plat("windows", "linux") then
    target("vulkan_surface_composition_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_surface_composition_test.cpp"))
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

    target("vulkan_device_preference_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_device_preference_test.cpp"))
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
    target("win32_window_backdrop_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/win32_window_backdrop_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "src/platform/win32"))
        add_syslinks("user32", "dwmapi", "advapi32")
        add_tests("default")

    target("win32_vulkan_backdrop_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/win32_vulkan_backdrop_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(path.join(os.projectdir(), public_includedirs))
        add_tests("default")
end

if is_plat("linux") then
    target("wayland_window_backdrop_test")
        set_kind("binary")
        add_files(
            path.join(os.projectdir(), "tests/platform/wayland_window_backdrop_test.cpp"),
            path.join(os.projectdir(), "tests/platform/wayland_test_background_effect.cpp"),
            path.join(os.projectdir(), "tests/platform/wayland_test_compositor.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland")
        add_packages("wayland")
        add_syslinks("wayland-server")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "src/platform/linux"),
            path.join(os.projectdir(), "tests/platform"))
        add_tests("default")
end
