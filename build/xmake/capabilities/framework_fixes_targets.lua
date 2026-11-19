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
