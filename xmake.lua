set_project("cgpui")
set_version("0.1.0")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

local public_includedirs = "include"

if is_plat("windows", "linux") then
    add_requires("vulkansdk")
end

if is_plat("linux") then
    add_requires("wayland")
end

target("cgpui_core")
    set_kind("static")
    add_files("src/core/*.cpp")
    add_includedirs(public_includedirs, {public = true})

target("cgpui_platform")
    set_kind("static")
    add_files("src/platform/*.cpp")
    remove_files("src/platform/platform_unimplemented.cpp")
    add_deps("cgpui_core")
    add_includedirs(public_includedirs, {public = true})

target("cgpui_platform_fallback")
    set_kind("static")
    add_files("src/platform/platform_unimplemented.cpp")
    add_deps("cgpui_core", "cgpui_platform")
    add_includedirs(public_includedirs, {public = true})

if is_plat("windows") then
    target("cgpui_platform_win32")
        set_kind("static")
        add_files("src/platform/win32/*.cpp")
        add_deps("cgpui_core", "cgpui_platform")
        add_includedirs(public_includedirs, {public = true})
        add_syslinks("user32", "gdi32", "shell32")
end

if is_plat("linux") then
    target("cgpui_platform_linux_wayland")
        set_kind("static")
        add_files("src/platform/linux/*.cpp")
        add_deps("cgpui_core", "cgpui_platform")
        add_packages("wayland")
        add_includedirs(public_includedirs, {public = true})
end

if is_plat("macosx") then
    target("cgpui_platform_macos")
        set_kind("static")
        add_files("src/platform/macos/*.mm")
        add_deps("cgpui_core", "cgpui_platform")
        add_frameworks("AppKit", "QuartzCore")
        add_includedirs(public_includedirs, {public = true})
end

target("cgpui_renderer")
    set_kind("static")
    add_files("src/renderer/*.cpp")
    remove_files("src/renderer/renderer_unimplemented.cpp")
    add_deps("cgpui_core")
    add_includedirs(public_includedirs, {public = true})

target("cgpui_renderer_fallback")
    set_kind("static")
    add_files("src/renderer/renderer_unimplemented.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
    add_includedirs(public_includedirs, {public = true})

if is_plat("windows", "linux") then
    target("cgpui_renderer_vulkan")
        set_kind("static")
        add_files("src/renderer/vulkan/*.cpp")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
        add_packages("vulkansdk")
        add_includedirs(public_includedirs, {public = true})
end

if is_plat("macosx") then
    target("cgpui_renderer_metal")
        set_kind("static")
        add_files("src/renderer/metal/*.mm")
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
        add_frameworks("Metal", "QuartzCore")
        add_includedirs(public_includedirs, {public = true})
end

target("core_header_cleanliness")
    set_kind("binary")
    add_files("tests/header_cleanliness/core_header_cleanliness.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default")

target("cgpui_ui")
    set_kind("static")
    add_files("src/ui/*.cpp")
    add_deps("cgpui_core", "cgpui_renderer")
    add_includedirs(public_includedirs, {public = true})

target("ui_header_cleanliness")
    set_kind("binary")
    add_files("tests/header_cleanliness/ui_header_cleanliness.cpp")
    add_deps("cgpui_core", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("render_view_test")
    set_kind("binary")
    add_files("tests/ui/render_view_test.cpp")
    add_deps("cgpui_core", "cgpui_renderer", "cgpui_ui")
    add_includedirs(public_includedirs)
    add_tests("default")

target("hello_window")
    set_kind("binary")
    add_files("examples/hello_window/main.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    if is_plat("windows") then
        add_deps("cgpui_platform_win32", "cgpui_renderer_vulkan")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_linux_wayland", "cgpui_renderer_vulkan")
    elseif is_plat("macosx") then
        add_deps("cgpui_platform_macos", "cgpui_renderer_metal")
        add_frameworks("AppKit", "QuartzCore", "Metal")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    add_includedirs(public_includedirs)
