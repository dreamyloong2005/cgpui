-- System appearance and DPI awareness on Wayland: xdg-desktop-portal
-- Settings over D-Bus, plus the scale-protocol awareness report.
local public_includedirs = "include"

target("wayland_system_appearance_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/wayland_system_appearance_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

if is_plat("linux") then
    target("wayland_portal_settings_test")
        set_kind("binary")
        add_files(
            path.join(os.projectdir(), "tests/platform/wayland_portal_settings_test.cpp"),
            path.join(os.projectdir(), "src/platform/linux/wayland_portal_settings.cpp"),
            path.join(os.projectdir(), "src/platform/linux/wayland_portal_settings_message.cpp"))
        add_deps("cgpui_core", "cgpui_platform")
        add_packages("dbus")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "src/platform/linux"),
            path.join(os.projectdir(), "tests/platform"))
        add_tests("default")

    target("wayland_portal_appearance_monitor_test")
        set_kind("binary")
        add_files(
            path.join(os.projectdir(), "tests/platform/wayland_portal_appearance_monitor_test.cpp"),
            path.join(os.projectdir(), "tests/platform/wayland_fake_portal.cpp"),
            path.join(os.projectdir(), "tests/platform/wayland_private_session_bus.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland")
        add_packages("dbus")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "src/platform/linux"),
            path.join(os.projectdir(), "tests/platform"))
        add_tests("default")

    target("wayland_dpi_awareness_test")
        set_kind("binary")
        add_files(
            path.join(os.projectdir(), "tests/platform/wayland_dpi_awareness_test.cpp"),
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
