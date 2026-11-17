-- In-app backdrop blur (C6): offscreen scene, downsample + separable
-- Gaussian, rounded-rect composite in the shared Vulkan renderer.
local public_includedirs = "include"

target("backdrop_blur_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/backdrop_blur_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("backdrop_blur_submission_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/backdrop_blur_submission_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs), path.join(os.projectdir(), "src/ui"))
    add_tests("default")

if is_plat("windows", "linux") then
    target("vulkan_backdrop_blur_plan_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_backdrop_blur_plan_test.cpp"))
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
    target("vulkan_backdrop_blur_pixel_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_backdrop_blur_pixel_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        add_packages("vulkansdk")
        add_includedirs(
            path.join(os.projectdir(), public_includedirs),
            path.join(os.projectdir(), "tests/renderer"))
        add_syslinks("user32")
        add_tests("default")
end

-- Built, not run: it needs a real window and a human to look at it.
if is_plat("windows", "linux") then
    target("example_backdrop_blur")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "examples/backdrop_blur/main.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui", "cgpui_app",
                 "cgpui_renderer_vulkan")
        if is_plat("windows") then
            add_deps("cgpui_platform_win32")
        else
            add_deps("cgpui_platform_linux_wayland")
        end
        add_includedirs(path.join(os.projectdir(), public_includedirs))
end
