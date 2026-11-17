-- Client title bar (C4): WM_NCCALCSIZE / WM_NCHITTEST on Windows,
-- client-side decorations with xdg_toplevel move/resize on Wayland, and
-- window control regions collected from painted elements.
local public_includedirs = "include"

target("client_title_bar_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/client_title_bar_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("core_window_title_bar_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/core_window_title_bar_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("platform_window_title_bar_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/platform_window_title_bar_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("ui_element_window_control_nodes_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/ui_element_window_control_nodes_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("window_control_region_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/window_control_region_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs), path.join(os.projectdir(), "src/ui"))
    add_tests("default")

if is_plat("windows") then
    target("win32_client_title_bar_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/win32_client_title_bar_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "src/platform/win32"))
        add_syslinks("user32", "dwmapi", "advapi32")
        add_tests("default")
end

if is_plat("linux") then
    target("wayland_client_title_bar_test")
        set_kind("binary")
        add_files(
            path.join(os.projectdir(), "tests/platform/wayland_client_title_bar_test.cpp"),
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

-- Built, not run: it needs a real window and a human to look at it.
if is_plat("windows", "linux") then
    target("example_client_title_bar")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "examples/client_title_bar/main.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app",
                 "cgpui_renderer_vulkan")
        if is_plat("windows") then
            add_deps("cgpui_platform_win32")
        else
            add_deps("cgpui_platform_linux_wayland")
        end
        add_includedirs(path.join(os.projectdir(), public_includedirs))
end
