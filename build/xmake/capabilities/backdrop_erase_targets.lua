-- Backdrop erase: with the DWM frame extended into the client area, the GDI
-- redirection surface is filled black (alpha 0) so the material shows
-- through transparent swapchain pixels.
local public_includedirs = "include"

target("backdrop_erase_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/backdrop_erase_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

if is_plat("windows") then
    target("win32_backdrop_erase_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/win32_backdrop_erase_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(path.join(os.projectdir(), public_includedirs))
        add_syslinks("user32", "gdi32", "dwmapi")
        add_tests("default")
end
