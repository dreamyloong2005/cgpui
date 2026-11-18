-- Native text shaping (C7): DirectWrite on Windows, HarfBuzz + FreeType +
-- fontconfig on Linux. Shaping, metrics, caches and glyph-id rasterization
-- are owned by cgpui_renderer next to the other text sources.
local public_includedirs = "include"

if is_plat("linux") then
    add_requires("harfbuzz", {system = true})
    add_requires("freetype", {system = true})
end

local shared_sources = {
    "src/ui/text_shaping_native.cpp",
    "src/ui/text_shaping_unicode.cpp",
    "src/ui/text_shape_cache.cpp",
    "src/ui/text_shaping_directwrite.cpp",
    "src/ui/text_font_defaults.cpp",
}
local windows_sources = {
    "src/ui/text_dwrite_fonts_win32.cpp",
    "src/ui/text_dwrite_analysis_win32.cpp",
    "src/ui/text_glyph_id_raster_win32.cpp",
}
local linux_sources = {
    "src/ui/text_fontconfig_faces_linux.cpp",
    "src/ui/text_glyph_id_raster_freetype.cpp",
}

target("cgpui_renderer")
    for _, source in ipairs(shared_sources) do
        add_files(path.join(os.projectdir(), source))
    end
    if is_plat("windows") then
        for _, source in ipairs(windows_sources) do
            add_files(path.join(os.projectdir(), source))
        end
    elseif is_plat("linux") then
        for _, source in ipairs(linux_sources) do
            add_files(path.join(os.projectdir(), source))
        end
        add_packages("harfbuzz", "freetype", "fontconfig", {public = true})
        add_defines("CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND",
                    "CGPUI_HAS_FREETYPE_GLYPH_RASTERIZER")
    end
target_end()

target("cgpui_ui")
    for _, list in ipairs({shared_sources, windows_sources, linux_sources}) do
        for _, source in ipairs(list) do
            remove_files(path.join(os.projectdir(), source))
        end
    end
target_end()

if is_plat("windows", "linux") then
    target("native_text_shaping_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/ui/native_text_shaping_test.cpp"),
                  path.join(os.projectdir(), "tests/ui/native_text_shaping_reference.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
        add_includedirs(path.join(os.projectdir(), public_includedirs),
                        path.join(os.projectdir(), "src/ui"))
        if is_plat("windows") then
            add_syslinks("dwrite")
        else
            add_packages("harfbuzz")
            add_defines("CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND")
        end
        add_tests("default")
end

if is_plat("windows", "linux") then
    target("native_text_glyph_spacing_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/native_text_glyph_spacing_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui",
                 "cgpui_renderer_vulkan")
        if is_plat("windows") then
            add_packages("vulkansdk")
        else
            add_syslinks("vulkan")
        end
        add_includedirs(path.join(os.projectdir(), public_includedirs))
        add_tests("default")
end

target("native_text_shaping_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/native_text_shaping_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("style_font_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/style_font_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("text_shape_cache_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/text_shape_cache_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("text_font_defaults_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/text_font_defaults_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")
