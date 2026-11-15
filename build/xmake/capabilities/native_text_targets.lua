-- Native text rasterization: renderer ownership and focused tests.
local public_includedirs = "include"

target("cgpui_renderer")
    add_files(path.join(os.projectdir(), "src/ui/text_glyph_rasterizer.cpp"))
    if is_plat("windows") then
        add_files(path.join(os.projectdir(), "src/ui/text_glyph_raster_win32.cpp"))
        add_files(path.join(os.projectdir(), "src/ui/text_glyph_coverage_win32.cpp"))
        add_syslinks("dwrite", {public = true})
    end
target_end()

target("cgpui_ui")
    remove_files(path.join(os.projectdir(), "src/ui/text_glyph_rasterizer.cpp"))
    remove_files(path.join(os.projectdir(), "src/ui/text_glyph_raster_win32.cpp"))
    remove_files(path.join(os.projectdir(), "src/ui/text_glyph_coverage_win32.cpp"))
target_end()

target("native_text_rasterizer_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/native_text_rasterizer_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("text_glyph_rasterizer_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/text_glyph_rasterizer_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

if is_plat("windows") then
    target("native_text_rasterization_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/native_text_rasterization_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(path.join(os.projectdir(), public_includedirs))
        add_tests("default")
end
