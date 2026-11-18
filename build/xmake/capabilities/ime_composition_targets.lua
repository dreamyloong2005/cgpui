-- IME composition caret and clause attributes (C8). IMM32 data is
-- converted by pure functions in win32_ime_composition_detail.cpp; the
-- text input widget draws clause underlines and the composition caret.
local public_includedirs = "include"

if is_plat("windows") then
    target("win32_ime_composition_detail_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/win32_ime_composition_detail_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(path.join(os.projectdir(), public_includedirs),
                        path.join(os.projectdir(), "src/platform/win32"))
        add_syslinks("user32", "imm32")
        add_tests("default")
end

target("text_composition_paint_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/text_composition_paint_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("ime_composition_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/ime_composition_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("text_composition_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/text_composition_header_cleanliness.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")
