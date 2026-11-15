-- System appearance: Win32 DWM/registry links and focused tests.
local public_includedirs = "include"

if is_plat("windows") then
    target("cgpui_platform_win32")
        add_syslinks("dwmapi", "advapi32", {public = true})
    target_end()
end

target("system_appearance_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/system_appearance_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("platform_appearance_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/platform_appearance_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("window_runtime_system_appearance_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/window_runtime_system_appearance_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

if is_plat("windows") then
    target("win32_system_appearance_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/win32_system_appearance_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(path.join(os.projectdir(), public_includedirs))
        add_syslinks("user32", "dwmapi", "advapi32")
        add_tests("default")
end
