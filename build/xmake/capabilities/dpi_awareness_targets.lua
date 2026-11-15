-- Per-Monitor V2 DPI awareness: focused tests for the Win32 declaration.
local public_includedirs = "include"

target("dpi_awareness_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/dpi_awareness_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("platform_dpi_awareness_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/platform_dpi_awareness_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

if is_plat("windows") then
    target("win32_dpi_awareness_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/win32_dpi_awareness_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(path.join(os.projectdir(), public_includedirs))
        add_syslinks("user32")
        add_tests("default")
end
