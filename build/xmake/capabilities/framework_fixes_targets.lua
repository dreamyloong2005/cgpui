-- Framework fixes found by the workbench work (C9): nested element
-- coordinates, secondary window element trees, Vulkan semaphores and UIA
-- patterns. Tests only; sources live in their existing targets.
local public_includedirs = "include"

target("element_nested_coordinates_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/element_nested_coordinates_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("view_element_tree_paint_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/view_element_tree_paint_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("stack_builder_style_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/stack_builder_style_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("scroll_element_offset_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/scroll_element_offset_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("flex_grow_relayout_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/flex_grow_relayout_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("window_record_action_dispatch_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/window_record_action_dispatch_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("framework_fixes_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/framework_fixes_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("window_record_element_tree_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/window_record_element_tree_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

if is_plat("windows", "linux") then
    target("vulkan_semaphore_validation_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/renderer/vulkan_semaphore_validation_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_renderer_vulkan")
        if is_plat("windows") then
            add_deps("cgpui_platform_win32")
            add_packages("vulkansdk")
            add_syslinks("user32", "gdi32")
        else
            add_deps("cgpui_platform_linux_wayland")
            add_packages("wayland")
            add_syslinks("vulkan")
        end
        add_includedirs(path.join(os.projectdir(), public_includedirs))
        add_tests("default")
end

if is_plat("windows") then
    target("win32_uia_item_pattern_test")
        set_kind("binary")
        add_files(path.join(os.projectdir(), "tests/platform/win32_uia_item_pattern_test.cpp"))
        add_deps("cgpui_core", "cgpui_platform", "cgpui_platform_win32")
        add_includedirs(path.join(os.projectdir(), public_includedirs),
                        path.join(os.projectdir(), "src/platform/win32"))
        add_syslinks("user32", "uiautomationcore", "oleaut32")
        add_tests("default")
end

target("workbench_accessibility_actions_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/workbench_accessibility_actions_test.cpp"))
    add_deps("cgpui_core", "cgpui_platform", "cgpui_renderer", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")

target("leaf_element_box_style_test")
    set_kind("binary")
    add_files(path.join(os.projectdir(), "tests/ui/leaf_element_box_style_test.cpp"))
    add_deps("cgpui_core", "cgpui_ui")
    add_includedirs(path.join(os.projectdir(), public_includedirs))
    add_tests("default")
