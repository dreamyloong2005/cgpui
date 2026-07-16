target("linux_backend_selection_test")
    set_kind("binary")
    add_files(
        path.join(os.projectdir(), "tests/platform/linux_backend_selection_test.cpp"),
        path.join(os.projectdir(), "src/platform/linux/linux_backend_selection.cpp"))
    add_deps("cgpui_core")
    add_includedirs(
        path.join(os.projectdir(), "include"),
        path.join(os.projectdir(), "src/platform/linux"))
    add_tests("default")

if is_plat("linux") then
    target("cgpui_platform_linux_x11")
        set_kind("static")
        add_files(path.join(os.projectdir(), "src/platform/linux/x11/*.cpp"))
        add_deps("cgpui_core", "cgpui_platform")
        add_packages("libxkbcommon")
        add_syslinks("xcb", "xcb-xkb", "xcb-cursor", "xkbcommon-x11", {public = true})
        add_includedirs(path.join(os.projectdir(), "include"), {public = true})

    target("cgpui_platform_linux_wayland")
        set_kind("static")
        add_files(path.join(os.projectdir(), "src/platform/linux/*.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_x11")
        add_packages("wayland", "dbus", "libxkbcommon")
        add_syslinks("wayland-cursor", {public = true})
        if has_package("fontconfig") then
            add_packages("fontconfig")
            add_defines("CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND")
        end
        add_includedirs(path.join(os.projectdir(), "include"), {public = true})

    target("x11_window_lifecycle_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/x11_window_lifecycle_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland")
        add_includedirs(path.join(os.projectdir(), "include"))
        add_tests("default")

    target("x11_scale_test")
        set_kind("binary")
        add_files(
            path.join(os.projectdir(), "tests/platform/x11_scale_test.cpp"),
            path.join(os.projectdir(), "src/platform/linux/x11/x11_scale.cpp"))
        add_deps("cgpui_core")
        add_syslinks("xcb")
        add_includedirs(
            path.join(os.projectdir(), "include"),
            path.join(os.projectdir(), "src/platform/linux/x11"))
        add_tests("default")

    target("x11_input_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/x11_input_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland")
        add_syslinks("xcb")
        add_includedirs(path.join(os.projectdir(), "include"))
        add_tests("default")

    target("x11_vulkan_surface_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/x11_vulkan_surface_test.cpp"))
        add_deps(
            "cgpui_core", "cgpui_platform", "cgpui_platform_linux_wayland",
            "cgpui_renderer", "cgpui_renderer_vulkan")
        add_syslinks("vulkan", "xcb")
        add_includedirs(path.join(os.projectdir(), "include"))
        add_tests("default")
end
