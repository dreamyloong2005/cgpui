-- Box shadow (C5): analytic Gaussian rounded-rect shadows in the shared
-- Vulkan renderer, submitted from PaintList / Style::box_shadow.
local public_includedirs = "include"

target("box_shadow_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/box_shadow_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("renderer_effect_draws_header_cleanliness")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/header_cleanliness/renderer_effect_draws_header_cleanliness.cpp"))
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("box_shadow_submission_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/box_shadow_submission_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs), path.join(os.projectdir(), "src/ui"))
    add_tests("default")

if is_plat("windows", "linux") then
    target("vulkan_box_shadow_geometry_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_box_shadow_geometry_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "src/renderer/vulkan"))
        if is_plat("windows") then
            add_packages("vulkansdk")
        else
            add_syslinks("vulkan")
        end
        add_tests("default")
end

if is_plat("windows") then
    target("vulkan_box_shadow_pixel_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_box_shadow_pixel_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "tests/renderer"))
        add_syslinks("user32")
        add_tests("default")
end
