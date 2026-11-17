-- Logical window size: WindowDescriptor::size is the logical client size,
-- converted with the window's DPI on Win32 (Per-Monitor V2).
local public_includedirs = "include"

target("logical_window_size_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/logical_window_size_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

if is_plat("windows") then
    target("win32_logical_window_size_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/win32_logical_window_size_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "src/platform/win32"))
        add_syslinks("user32")
        add_tests("default")
end
