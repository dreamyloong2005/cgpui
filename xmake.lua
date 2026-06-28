set_project("cgpui")
set_version("0.1.0")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

local public_includedirs = "include"

target("cgpui_core")
    set_kind("static")
    add_files("src/core/*.cpp")
    add_includedirs(public_includedirs, {public = true})

target("cgpui_platform")
    set_kind("static")
    add_files("src/platform/*.cpp")
    add_deps("cgpui_core")
    add_includedirs(public_includedirs, {public = true})

target("cgpui_renderer")
    set_kind("static")
    add_files("src/renderer/*.cpp")
    add_deps("cgpui_core")
    add_includedirs(public_includedirs, {public = true})

target("core_header_cleanliness")
    set_kind("binary")
    add_files("tests/header_cleanliness/core_header_cleanliness.cpp")
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer")
    add_includedirs(public_includedirs)
    add_tests("default")
