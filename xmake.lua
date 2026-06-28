set_project("cgpui")
set_version("0.1.0")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

local public_includedirs = "include"

if is_plat("windows", "linux") then
    add_requires("vulkansdk")
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
        add_deps("cgpui_platform_win32")
    elseif is_plat("linux") then
        add_deps("cgpui_platform_fallback")
    else
        add_deps("cgpui_platform_fallback", "cgpui_renderer_fallback")
    end
    if is_plat("windows", "linux") then
        add_deps("cgpui_renderer_vulkan")
    end
    add_includedirs(public_includedirs)
